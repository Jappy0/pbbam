// Author: Derek Barnett

#include "PbbamInternalConfig.h"

#include "FormatUtils.h"

#include <algorithm>

#include <boost/algorithm/string.hpp>

#include "MemoryUtils.h"

namespace PacBio {
namespace BAM {

const std::vector<std::string>& FormatUtils::FastaExtensions()
{
    static const std::vector<std::string> extensions{"fa", "fasta", "fa.gz", "fasta.gz"};
    return extensions;
}

const std::vector<std::string>& FormatUtils::FastqExtensions()
{
    static const std::vector<std::string> extensions{"fq", "fastq", "fq.gz", "fastq.gz"};
    return extensions;
}

HtslibCompression FormatUtils::CompressionType(BGZF* bgzf)
{
    return static_cast<HtslibCompression>(bgzf_compression(bgzf));
}

HtslibCompression FormatUtils::CompressionType(const std::string& fn)
{
    std::unique_ptr<BGZF, HtslibBgzfDeleter> bgzf(bgzf_open(fn.c_str(), "rb"));
    if (bgzf == nullptr) throw std::runtime_error{"could not check compression level for: " + fn};
    return CompressionType(bgzf.get());
}

bool FormatUtils::IsFastaFilename(const std::string& fn) { return IsFormat(fn, FastaExtensions()); }

bool FormatUtils::IsFastqFilename(const std::string& fn) { return IsFormat(fn, FastqExtensions()); }

bool FormatUtils::IsFormat(const std::string& fn, const std::vector<std::string>& extensions)
{
    const auto found = std::find_if(
        extensions.cbegin(), extensions.cend(),
        [&fn](const std::string& ext) { return boost::algorithm::iends_with(fn, ext); });
    return found != extensions.cend();
}

}  // namespace BAM
}  // namespace PacBio
