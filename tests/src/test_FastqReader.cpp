// Author: Derek Barnett

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>

#include "PbbamTestData.h"

#include <pbbam/BamFileMerger.h>
#include <pbbam/EntireFileQuery.h>
#include <pbbam/FastqReader.h>
#include <pbbam/FastqSequence.h>
#include <pbbam/FastqWriter.h>

#include "FastxTests.h"

using namespace PacBio;
using namespace PacBio::BAM;

namespace FastqReaderTests {

void CheckFastqSequence(const size_t index, const FastqSequence& seq)
{
    SCOPED_TRACE("checking FASTA seq:" + std::to_string(index));
    const auto& expected = FastxTests::ExpectedFastq.at(index);
    EXPECT_EQ(expected.Name(), seq.Name());
    EXPECT_EQ(expected.Bases(), seq.Bases());
    EXPECT_EQ(expected.Qualities().Fastq(), seq.Qualities().Fastq());
}

}  // namespace FastqReaderTests

TEST(FastqReaderTest, throws_on_empty_filename)
{
    EXPECT_THROW(FastqReader reader{""}, std::runtime_error);
}

TEST(FastqReaderTest, throws_on_invalid_extension)
{
    EXPECT_THROW(FastqReader reader{"wrong.ext"}, std::runtime_error);
}

TEST(FastqReaderTest, can_open_text_fastq)
{
    const auto& fn = FastxTests::simpleFastqFn;
    EXPECT_NO_THROW(FastqReader reader{fn});
}

TEST(FastqReaderTest, can_open_gzip_fastq)
{
    const auto& fn = FastxTests::simpleFastqGzipFn;
    EXPECT_NO_THROW(FastqReader reader{fn});
}

TEST(FastqReaderTest, can_open_bgzf_fastq)
{
    const auto& fn = FastxTests::simpleFastqBgzfFn;
    EXPECT_NO_THROW(FastqReader reader{fn});
}

TEST(FastqReaderTest, can_iterate_manually_on_text_fastq)
{
    const auto& fn = FastxTests::simpleFastqFn;
    FastqReader reader{fn};

    size_t count = 0;
    FastqSequence seq;
    while (reader.GetNext(seq)) {
        FastqReaderTests::CheckFastqSequence(count, seq);
        ++count;
    }
    EXPECT_EQ(FastxTests::ExpectedFastq.size(), count);
}

TEST(FastqReaderTest, can_iterate_manually_on_gzip_fastq)
{
    const auto& fn = FastxTests::simpleFastqGzipFn;
    FastqReader reader{fn};

    size_t count = 0;
    FastqSequence seq;
    while (reader.GetNext(seq)) {
        FastqReaderTests::CheckFastqSequence(count, seq);
        ++count;
    }
    EXPECT_EQ(FastxTests::ExpectedFastq.size(), count);
}

TEST(FastqReaderTest, can_iterate_manually_on_bgzf_fastq)
{
    const auto& fn = FastxTests::simpleFastqBgzfFn;
    FastqReader reader{fn};

    size_t count = 0;
    FastqSequence seq;
    while (reader.GetNext(seq)) {
        FastqReaderTests::CheckFastqSequence(count, seq);
        ++count;
    }
    EXPECT_EQ(FastxTests::ExpectedFastq.size(), count);
}

TEST(FastqReaderTest, can_iterate_using_range_for_on_text_fastq)
{
    const auto& fn = FastxTests::simpleFastqFn;

    size_t count = 0;
    FastqReader reader{fn};
    for (const auto& seq : reader) {
        FastqReaderTests::CheckFastqSequence(count, seq);
        ++count;
    }
    EXPECT_EQ(FastxTests::ExpectedFastq.size(), count);
}

TEST(FastqReaderTest, can_iterate_using_range_for_on_gzip_fastq)
{
    const auto& fn = FastxTests::simpleFastqGzipFn;

    size_t count = 0;
    FastqReader reader{fn};
    for (const auto& seq : reader) {
        FastqReaderTests::CheckFastqSequence(count, seq);
        ++count;
    }
    EXPECT_EQ(FastxTests::ExpectedFastq.size(), count);
}

TEST(FastqReaderTest, can_iterate_using_range_for_on_bgzf_fastq)
{
    const auto& fn = FastxTests::simpleFastqBgzfFn;

    size_t count = 0;
    FastqReader reader{fn};
    for (const auto& seq : reader) {
        FastqReaderTests::CheckFastqSequence(count, seq);
        ++count;
    }
    EXPECT_EQ(FastxTests::ExpectedFastq.size(), count);
}

TEST(FastqReaderTest, can_read_all_from_text_fastq)
{
    const auto& fn = FastxTests::simpleFastqFn;

    size_t count = 0;
    for (const auto& seq : FastqReader::ReadAll(fn)) {
        FastqReaderTests::CheckFastqSequence(count, seq);
        ++count;
    }
    EXPECT_EQ(FastxTests::ExpectedFastq.size(), count);
}

TEST(FastqReaderTest, can_read_all_from_gzip_fastq)
{
    const auto& fn = FastxTests::simpleFastqGzipFn;

    size_t count = 0;
    for (const auto& seq : FastqReader::ReadAll(fn)) {
        FastqReaderTests::CheckFastqSequence(count, seq);
        ++count;
    }
    EXPECT_EQ(FastxTests::ExpectedFastq.size(), count);
}

TEST(FastqReaderTest, can_read_all_from_bgzf_fastq)
{
    const auto& fn = FastxTests::simpleFastqBgzfFn;

    size_t count = 0;
    for (const auto& seq : FastqReader::ReadAll(fn)) {
        FastqReaderTests::CheckFastqSequence(count, seq);
        ++count;
    }
    EXPECT_EQ(FastxTests::ExpectedFastq.size(), count);
}

TEST(FastqReaderTest, can_handle_windows_style_newlines)
{
    const std::string fn = FastxTests::fastxDataDir + "/windows_formatted.fastq";

    {
        FastqReader reader{fn};
        FastqSequence seq;
        reader.GetNext(seq);  // 1 sequence in total
        EXPECT_EQ("C5", seq.Name());
        EXPECT_EQ("AAGCA", seq.Bases());
        EXPECT_EQ("~~~~~", seq.Qualities().Fastq());
    }
}

TEST(FastqMerging, can_merge_bams_to_fastq_output)
{
    const std::vector<std::string> bamFiles{PbbamTestsConfig::Data_Dir + "/group/test1.bam",
                                            PbbamTestsConfig::Data_Dir + "/group/test2.bam",
                                            PbbamTestsConfig::Data_Dir + "/group/test3.bam"};

    const std::string outFastq = PbbamTestsConfig::GeneratedData_Dir + "/out.fq";

    {
        FastqWriter fastq{outFastq};
        BamFileMerger::Merge(bamFiles, fastq);
    }

    const std::vector<std::string> mergedFastqNames{
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/14743/2114_2531",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/14743/2579_4055",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/14743/4101_5571",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/14743/5615_6237",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/24962/0_427",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/45203/0_893",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/45203/0_893",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/46835/3759_4005",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/46835/4052_4686",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/46835/4732_4869",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/47698/9482_9628",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/47698/9675_10333",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/47698/10378_10609",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/49050/48_1132",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/49050/48_1132",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/49194/0_798",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/49194/845_1541",
        "m140905_042212_sidney_c100564852550000001823085912221377_s1_X0/49521/0_134"};

    const auto seqs = FastqReader::ReadAll(outFastq);
    ASSERT_EQ(mergedFastqNames.size(), seqs.size());

    for (size_t i = 0; i < seqs.size(); ++i)
        EXPECT_EQ(mergedFastqNames[i], seqs[i].Name());

    remove(outFastq.c_str());
}