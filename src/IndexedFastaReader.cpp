// File Description
/// \file IndexedFastaReader.cpp
/// \brief Implements the IndexedFastaReader class.
//
// Author: David Alexander

#include "PbbamInternalConfig.h"

#include "pbbam/IndexedFastaReader.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <memory>

#include <htslib/faidx.h>

#include "SequenceUtils.h"
#include "pbbam/BamRecord.h"
#include "pbbam/GenomicInterval.h"
#include "pbbam/Orientation.h"
#include "pbbam/StringUtilities.h"

namespace PacBio {
namespace BAM {

namespace {

void ClipAndGapify(std::string& subseq, const Cigar& cigar, bool exciseSoftClips)
{
    size_t seqIndex = 0;
    for (const auto& op : cigar) {
        const auto type = op.Type();
        const auto opLength = op.Length();

        // do nothing for hard clips
        if (type == CigarOperationType::HARD_CLIP) continue;

        // maybe remove soft clips
        if (type == CigarOperationType::SOFT_CLIP) {
            if (!exciseSoftClips) {
                subseq.reserve(subseq.size() + opLength);
                subseq.insert(seqIndex, opLength, '-');
                seqIndex += opLength;
            }
        }

        // for non-clipping operations
        else {
            // maybe add gaps/padding
            if (type == CigarOperationType::INSERTION) {
                subseq.reserve(subseq.size() + opLength);
                subseq.insert(seqIndex, opLength, '-');
            } else if (type == CigarOperationType::PADDING) {
                subseq.reserve(subseq.size() + opLength);
                subseq.insert(seqIndex, opLength, '*');
            }

            // update index
            seqIndex += opLength;
        }
    }
}

struct FreeDeleter
{
    // Need to deallocate the returned pointer from htslib using `free()`,
    // as `delete`-deallocating a pointer originally allocated with `malloc`
    // constitutes undefined behavior and ASAN rightfully errors out
    // with a `alloc-dealloc-mismatch` message.
    // See also:
    //   https://github.com/samtools/htslib/blob/develop/htslib/faidx.h#L195
    void operator()(char* p) const { std::free(p); }
};

}  // anonymous

IndexedFastaReader::IndexedFastaReader(const std::string& filename)
{
    if (!Open(filename)) throw std::runtime_error{"Cannot open file " + filename};
}

IndexedFastaReader::IndexedFastaReader(const IndexedFastaReader& src)
{
    if (!Open(src.filename_)) throw std::runtime_error{"Cannot open file " + src.filename_};
}

IndexedFastaReader::IndexedFastaReader(IndexedFastaReader&&) = default;

IndexedFastaReader& IndexedFastaReader::operator=(const IndexedFastaReader& rhs)
{
    if (&rhs == this) return *this;

    Open(rhs.filename_);
    return *this;
}

IndexedFastaReader& IndexedFastaReader::operator=(IndexedFastaReader&&) = default;

IndexedFastaReader::~IndexedFastaReader() { Close(); }

bool IndexedFastaReader::Open(std::string filename)
{
    auto* handle = fai_load(filename.c_str());
    if (handle == nullptr)
        return false;
    else {
        filename_ = std::move(filename);
        handle_ = handle;
        return true;
    }
}

void IndexedFastaReader::Close()
{
    filename_.clear();
    if (handle_ != nullptr) fai_destroy(handle_);
    handle_ = nullptr;
}

#define REQUIRE_FAIDX_LOADED                     \
    if (handle_ == nullptr) throw std::exception \
        {                                        \
        }

std::string IndexedFastaReader::Subsequence(const std::string& id, Position begin,
                                            Position end) const
{
    REQUIRE_FAIDX_LOADED;

    assert(begin <= end);
    // htslib is dumb and will not consider empty intervals valid,
    // that is, a call to faidx_fetch_seq will *always* return a
    // sequence consisting of at least one base.
    if (begin == end) return std::string{};

    int len;
    // Derek: *Annoyingly* htslib seems to interpret "end" as inclusive in
    // faidx_fetch_seq, whereas it considers it exclusive in the region spec in
    // fai_fetch.  Can you please verify?
    const std::unique_ptr<char, FreeDeleter> rawSeq{
        faidx_fetch_seq(handle_, id.c_str(), begin, end - 1, &len)};
    if (rawSeq == nullptr) throw std::runtime_error{"could not fetch FASTA sequence"};
    return RemoveAllWhitespace(rawSeq.get());
}

std::string IndexedFastaReader::Subsequence(const GenomicInterval& interval) const
{
    REQUIRE_FAIDX_LOADED;
    return Subsequence(interval.Name(), interval.Start(), interval.Stop());
}

std::string IndexedFastaReader::Subsequence(const char* htslibRegion) const
{
    REQUIRE_FAIDX_LOADED;

    int len;
    const std::unique_ptr<char, FreeDeleter> rawSeq(fai_fetch(handle_, htslibRegion, &len));
    if (rawSeq == nullptr) throw std::runtime_error{"could not fetch FASTA sequence"};
    return RemoveAllWhitespace(rawSeq.get());
}

std::string IndexedFastaReader::ReferenceSubsequence(const BamRecord& bamRecord,
                                                     const Orientation orientation,
                                                     const bool gapped,
                                                     const bool exciseSoftClips) const
{
    REQUIRE_FAIDX_LOADED;

    std::string subseq = Subsequence(bamRecord.ReferenceName(), bamRecord.ReferenceStart(),
                                     bamRecord.ReferenceEnd());

    if (bamRecord.Impl().IsMapped() && gapped)
        ClipAndGapify(subseq, bamRecord.Impl().CigarData(), exciseSoftClips);

    const auto reverse =
        (orientation != Orientation::GENOMIC) && bamRecord.Impl().IsReverseStrand();
    if (reverse) ReverseComplementCaseSens(subseq);

    return subseq;
}

int IndexedFastaReader::NumSequences() const
{
    REQUIRE_FAIDX_LOADED;
    return faidx_nseq(handle_);
}

std::vector<std::string> IndexedFastaReader::Names() const
{
    REQUIRE_FAIDX_LOADED;
    std::vector<std::string> names;
    names.reserve(NumSequences());
    for (int i = 0; i < NumSequences(); ++i)
        names.emplace_back(faidx_iseq(handle_, i));
    return names;
}

std::string IndexedFastaReader::Name(const size_t idx) const
{
    REQUIRE_FAIDX_LOADED;
    if (static_cast<int>(idx) >= NumSequences())
        throw std::runtime_error{"FASTA index out of range"};
    return {faidx_iseq(handle_, idx)};
}

bool IndexedFastaReader::HasSequence(const std::string& name) const
{
    REQUIRE_FAIDX_LOADED;
    return (faidx_has_seq(handle_, name.c_str()) != 0);
}

int IndexedFastaReader::SequenceLength(const std::string& name) const
{
    REQUIRE_FAIDX_LOADED;
    const auto len = faidx_seq_len(handle_, name.c_str());
    if (len < 0)
        throw std::runtime_error{"could not determine FASTA sequence length"};
    else
        return len;
}
}
}  // PacBio::BAM
