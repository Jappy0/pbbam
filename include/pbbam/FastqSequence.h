// File Description
/// \file FastqSequence.h
/// \brief Defines the FastqSequence class.
//
// Author: Derek Barnett

#ifndef FASTQSEQUENCE_H
#define FASTQSEQUENCE_H

#include <pbbam/FastaSequence.h>
#include <pbbam/QualityValues.h>

#include <string>

namespace PacBio {
namespace BAM {

///
/// \brief The FastqSequence class represents a FASTQ record (name, bases, and
///        qualities)
///
class FastqSequence : public FastaSequence
{
public:
    /// \name Constructors & Related Methods
    /// \{

    ///
    /// \brief FastaSequence
    /// \param name
    /// \param bases
    /// \param qualities
    ///
    explicit FastqSequence(std::string name, std::string bases, QualityValues qualities);

    ///
    /// \brief FastaSequence
    /// \param name
    /// \param bases
    /// \param qualities
    ///
    explicit FastqSequence(std::string name, std::string bases, std::string qualities);

    FastqSequence();
    FastqSequence(const FastqSequence&);
    FastqSequence(FastqSequence&&);
    FastqSequence& operator=(const FastqSequence&);
    FastqSequence& operator=(FastqSequence&&);
    ~FastqSequence();

    /// \}

public:
    /// \name Attributes
    /// \{

    ///
    /// \brief Qualities
    /// \return
    ///
    const QualityValues& Qualities() const;

    /// \}

private:
    QualityValues qualities_;
};

}  // namespace BAM
}  // namespace PacBio

#endif  // FASTQSEQUENCE_H
