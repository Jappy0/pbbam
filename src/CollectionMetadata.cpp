#include "PbbamInternalConfig.h"

#include <pbbam/CollectionMetadata.h>

#include "DataSetUtils.h"
#include "RunMetadataParser.h"
#include "pugixml/pugixml.hpp"

#include <boost/algorithm/string/predicate.hpp>

#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace PacBio {
namespace BAM {
namespace {

std::optional<ControlKit::CustomSequence> UpdateControlKitCache(const ControlKit& kit)
{
    if (!kit.HasChild("CustomSequence")) {
        return {};
    }

    const auto& customSeq = kit.ChildText("CustomSequence");
    const auto lines = [](const std::string& input) {
        std::vector<std::string> result;
        size_t pos = 0;
        size_t found = input.find("\\n");
        while (found != std::string::npos) {
            result.push_back(input.substr(pos, found - pos));
            pos = found + 2;  // "\n"
            found = input.find("\\n", pos);
        }
        result.push_back(input.substr(pos));  // store last
        return result;
    }(customSeq);

    if (lines.size() != 6) {
        throw std::runtime_error{"[pbbam] run metadata ERROR: malformatted CustomSequence node"};
    }

    return ControlKit::CustomSequence{lines.at(1), lines.at(3), lines.at(5)};
}

void UpdateControlKit(const std::optional<ControlKit::CustomSequence>& cache, ControlKit& kit)
{
    std::ostringstream seq;
    seq << ">left_adapter\\n"
        << cache->LeftAdapter << "\\n"
        << ">right_adapter\\n"
        << cache->RightAdapter << "\\n"
        << ">custom_sequence\\n"
        << cache->Sequence;
    kit.ChildText("CustomSequence", seq.str());
}

void CollectionMetadataElementFromXml(const pugi::xml_node& xmlNode,
                                      internal::DataSetElement& parent)
{
    const std::string label = xmlNode.name();
    if (label.empty()) {
        return;
    }

    // TODO(DB): This is getting a bit 'hacky'. Should revisit namespace-
    //           handling internally.
    ///
    // ensure 'pbmeta' namespace for child elements, except for:
    //  - 'AutomationParameter' & 'AutomationParameters' which are 'pbbase'
    //  - 'BioSample' & 'BioSamples' which are 'pbsample'
    ///
    const XsdType xsdType = [&]() {
        if (label.find("BioSample") != std::string::npos) {
            return XsdType::SAMPLE_INFO;
        } else if (label.find("AutomationParameter") != std::string::npos) {
            return XsdType::BASE_DATA_MODEL;
        } else {
            return XsdType::COLLECTION_METADATA;
        }
    }();

    internal::DataSetElement e{label, xsdType};
    e.Text(xmlNode.text().get());

    // iterate attributes
    auto attrIter = xmlNode.attributes_begin();
    auto attrEnd = xmlNode.attributes_end();
    for (; attrIter != attrEnd; ++attrIter) {
        e.Attribute(attrIter->name(), attrIter->value());
    }

    // iterate children, recursively building up subtree
    auto childIter = xmlNode.begin();
    auto childEnd = xmlNode.end();
    for (; childIter != childEnd; ++childIter) {
        pugi::xml_node childNode = *childIter;
        CollectionMetadataElementFromXml(childNode, e);
    }

    parent.AddChild(e);
}

}  // namespace

// ----------------------
// Automation
// ----------------------

Automation::Automation() : internal::DataSetElement{"Automation", XsdType::COLLECTION_METADATA} {}

Automation::Automation(const internal::FromInputXml& fromInputXml)
    : internal::DataSetElement{"", fromInputXml, XsdType::COLLECTION_METADATA}
{}

DEFINE_ACCESSORS(Automation, AutomationParameters, AutomationParameters)

Automation& Automation::AutomationParameters(BAM::AutomationParameters params)
{
    AutomationParameters() = params;
    return *this;
}
bool Automation::HasAutomationParameters() const
{
    return HasChild(Element::AUTOMATION_PARAMETERS);
}

// ----------------------
// AutomationParameter
// ----------------------

AutomationParameter::AutomationParameter()
    : internal::DataSetElement{"AutomationParameter", XsdType::BASE_DATA_MODEL}
{}

AutomationParameter::AutomationParameter(const internal::FromInputXml& fromInputXml)
    : internal::DataSetElement{"", fromInputXml, XsdType::BASE_DATA_MODEL}
{}

AutomationParameter::AutomationParameter(const std::string& name, const std::string& type,
                                         const std::string& value)
    : internal::DataSetElement{"AutomationParameter", XsdType::BASE_DATA_MODEL}
{
    Name(name);
    Type(type);
    Value(value);
}

AutomationParameter::AutomationParameter(const std::string& name, const std::string& type,
                                         const std::string& value,
                                         const internal::FromInputXml& fromInputXml)
    : internal::DataSetElement{"", fromInputXml, XsdType::COLLECTION_METADATA}
{
    Name(name);
    Type(type);
    Value(value);
}

const std::string& AutomationParameter::Name() const { return Attribute("Name"); }
std::string& AutomationParameter::Name() { return Attribute("Name"); }
AutomationParameter& AutomationParameter::Name(const std::string& name)
{
    Attribute("Name") = name;
    return *this;
}

const std::string& AutomationParameter::Type() const { return Attribute("ValueDataType"); }
std::string& AutomationParameter::Type() { return Attribute("ValueDataType"); }
AutomationParameter& AutomationParameter::Type(const std::string& type)
{
    Attribute("ValueDataType") = type;
    return *this;
}

const std::string& AutomationParameter::Value() const { return Attribute("SimpleValue"); }
std::string& AutomationParameter::Value() { return Attribute("SimpleValue"); }
AutomationParameter& AutomationParameter::Value(const std::string& value)
{
    Attribute("SimpleValue") = value;
    return *this;
}

// ----------------------
// AutomationParameters
// ----------------------

AutomationParameters::AutomationParameters()
    : internal::DataSetElement{"AutomationParameters", XsdType::BASE_DATA_MODEL}
{}
AutomationParameters::AutomationParameters(const internal::FromInputXml& fromInputXml)
    : internal::DataSetElement{"", fromInputXml, XsdType::BASE_DATA_MODEL}
{}

AutomationParameters::iterator_type AutomationParameters::begin()
{
    return AutomationParameters::iterator_type(this, 0);
}

AutomationParameters::const_iterator_type AutomationParameters::begin() const { return cbegin(); }

AutomationParameters::const_iterator_type AutomationParameters::cbegin() const
{
    return AutomationParameters::const_iterator_type(this, 0);
}

AutomationParameters::iterator_type AutomationParameters::end()
{
    return AutomationParameters::iterator_type(this, NumChildren());
}

AutomationParameters::const_iterator_type AutomationParameters::end() const { return cend(); }

AutomationParameters::const_iterator_type AutomationParameters::cend() const
{
    return AutomationParameters::const_iterator_type(this, NumChildren());
}

int32_t AutomationParameters::CellNFCIndex() const
{
    return std::stoi(GetParameter(Element::CELL_NFC_INDEX));
}

AutomationParameters& AutomationParameters::CellNFCIndex(int32_t i)
{
    return SetParameter(Element::CELL_NFC_INDEX, "Int32", std::to_string(i));
}

bool AutomationParameters::HasCellNFCIndex() const { return HasParameter(Element::CELL_NFC_INDEX); }

int32_t AutomationParameters::CollectionNumber() const
{
    return std::stoi(GetParameter(Element::COLLECTION_NUMBER));
}

AutomationParameters& AutomationParameters::CollectionNumber(int32_t i)
{
    return SetParameter(Element::COLLECTION_NUMBER, "Int32", std::to_string(i));
}

bool AutomationParameters::HasCollectionNumber() const
{
    return HasParameter(Element::CELL_NFC_INDEX);
}

double AutomationParameters::Exposure() const { return std::stod(GetParameter(Element::EXPOSURE)); }

AutomationParameters& AutomationParameters::Exposure(double d)
{
    return SetParameter(Element::EXPOSURE, "Double", std::to_string(d));
}

bool AutomationParameters::HasExposure() const { return HasParameter(Element::EXPOSURE); }

bool AutomationParameters::ExtendFirst() const
{
    return boost::iequals(GetParameter(Element::EXTEND_FIRST), "True");
}

AutomationParameters& AutomationParameters::ExtendFirst(bool ok)
{
    return SetParameter(Element::EXTEND_FIRST, "Boolean", (ok ? "True" : "False"));
}

bool AutomationParameters::HasExtendFirst() const { return HasParameter(Element::EXTEND_FIRST); }

double AutomationParameters::ExtensionTime() const
{
    return std::stod(GetParameter(Element::EXTENSION_TIME));
}

AutomationParameters& AutomationParameters::ExtensionTime(double d)
{
    return SetParameter(Element::EXTENSION_TIME, "Double", std::to_string(d));
}

bool AutomationParameters::HasExtensionTime() const
{
    return HasParameter(Element::EXTENSION_TIME);
}

int32_t AutomationParameters::ExtraIMWashes() const
{
    return std::stoi(GetParameter(Element::EXTRA_IM_WASHES));
}

AutomationParameters& AutomationParameters::ExtraIMWashes(int32_t i)
{
    return SetParameter(Element::EXTRA_IM_WASHES, "Int32", std::to_string(i));
}

bool AutomationParameters::HasExtraIMWashes() const
{
    return HasParameter(Element::EXTRA_IM_WASHES);
}

bool AutomationParameters::HasN2Switch() const
{
    return boost::iequals(GetParameter(Element::HAS_N2_SWITCH), "True");
}

AutomationParameters& AutomationParameters::HasN2Switch(bool ok)
{
    return SetParameter(Element::HAS_N2_SWITCH, "Boolean", (ok ? "True" : "False"));
}

bool AutomationParameters::HasHasN2Switch() const { return HasParameter(Element::HAS_N2_SWITCH); }

std::string AutomationParameters::HQRFMethod() const { return GetParameter(Element::HQRF_METHOD); }

AutomationParameters& AutomationParameters::HQRFMethod(std::string s)
{
    return SetParameter(Element::HQRF_METHOD, "String", s);
}

bool AutomationParameters::HasHQRFMethod() const { return HasParameter(Element::HQRF_METHOD); }

double AutomationParameters::ImmobilizationTime() const
{
    return std::stod(GetParameter(Element::IMMOBILIZATION_TIME));
}

AutomationParameters& AutomationParameters::ImmobilizationTime(double d)
{
    return SetParameter(Element::IMMOBILIZATION_TIME, "Double", std::to_string(d));
}

bool AutomationParameters::HasImmobilizationTime() const
{
    return HasParameter(Element::IMMOBILIZATION_TIME);
}

int32_t AutomationParameters::InsertSize() const
{
    return std::stoi(GetParameter(Element::INSERT_SIZE));
}

AutomationParameters& AutomationParameters::InsertSize(int32_t i)
{
    return SetParameter(Element::INSERT_SIZE, "Int32", std::to_string(i));
}

bool AutomationParameters::HasInsertSize() const { return HasParameter(Element::INSERT_SIZE); }

double AutomationParameters::MovieLength() const
{
    return std::stod(GetParameter(Element::MOVIE_LENGTH));
}

AutomationParameters& AutomationParameters::MovieLength(double d)
{
    return SetParameter(Element::IMMOBILIZATION_TIME, "Double", std::to_string(d));
}

bool AutomationParameters::HasMovieLength() const { return HasParameter(Element::MOVIE_LENGTH); }

bool AutomationParameters::PCDinPlate() const
{
    return boost::iequals(GetParameter(Element::PCD_IN_PLATE), "True");
}

AutomationParameters& AutomationParameters::PCDinPlate(bool ok)
{
    return SetParameter(Element::PCD_IN_PLATE, "Boolean", (ok ? "True" : "False"));
}

bool AutomationParameters::HasPCDinPlate() const { return HasParameter(Element::PCD_IN_PLATE); }

bool AutomationParameters::PreExtensionWorkflow() const
{
    return boost::iequals(GetParameter(Element::PRE_EXTENSION_WORKFLOW), "True");
}

AutomationParameters& AutomationParameters::PreExtensionWorkflow(bool ok)
{
    return SetParameter(Element::PRE_EXTENSION_WORKFLOW, "Boolean", (ok ? "True" : "False"));
}

bool AutomationParameters::HasPreExtensionWorkflow() const
{
    return HasParameter(Element::PRE_EXTENSION_WORKFLOW);
}

double AutomationParameters::SNRCut() const { return std::stod(GetParameter(Element::SNR_CUT)); }

AutomationParameters& AutomationParameters::SNRCut(double d)
{
    return SetParameter(Element::SNR_CUT, "Double", std::to_string(d));
}

bool AutomationParameters::HasSNRCut() const { return HasParameter(Element::SNR_CUT); }

int32_t AutomationParameters::TipSearchMaxDuration() const
{
    return std::stoi(GetParameter(Element::TIP_SEARCH_MAX_DURATION));
}

AutomationParameters& AutomationParameters::TipSearchMaxDuration(int32_t i)
{
    return SetParameter(Element::TIP_SEARCH_MAX_DURATION, "Int32", std::to_string(i));
}

bool AutomationParameters::HasTipSearchMaxDuration() const
{
    return HasParameter(Element::TIP_SEARCH_MAX_DURATION);
}

bool AutomationParameters::UseStageHotStart() const
{
    return boost::iequals(GetParameter(Element::USE_STAGE_HOT_START), "True");
}

AutomationParameters& AutomationParameters::UseStageHotStart(bool ok)
{
    return SetParameter(Element::USE_STAGE_HOT_START, "Boolean", (ok ? "True" : "False"));
    return *this;
}

bool AutomationParameters::HasUseStageHotStart() const
{
    return HasParameter(Element::USE_STAGE_HOT_START);
}

std::string AutomationParameters::GetParameter(const std::string& param) const
{
    const size_t count = NumChildren();
    for (size_t i = 0; i < count; ++i) {
        const internal::DataSetElement& child = *(children_.at(i).get());
        if (child.Attribute("Name") == param) {
            return child.Attribute("SimpleValue");
        }
    }

    throw std::runtime_error{""};
}

AutomationParameters& AutomationParameters::SetParameter(const std::string& name,
                                                         const std::string& type,
                                                         const std::string& value)
{
    const size_t count = NumChildren();
    for (size_t i = 0; i < count; ++i) {
        internal::DataSetElement* child = children_.at(i).get();
        if (child->Attribute("Name") == name) {
            child->Attribute("ValueDataType", type);
            child->Attribute("SimpleValue", value);
            return *this;
        }
    }

    // not found
    AddChild(AutomationParameter{name, type, value, internal::FromInputXml{}});
    return *this;
}

bool AutomationParameters::HasParameter(const std::string& param) const
{
    const size_t count = NumChildren();
    for (size_t i = 0; i < count; ++i) {
        const internal::DataSetElement* child = children_.at(i).get();
        if (child->Attribute("Name") == param) {
            return true;
        }
    }

    return false;
}

// ----------------------
// BindingKit
// ----------------------

BindingKit::BindingKit() : internal::DataSetElement{"BindingKit", XsdType::COLLECTION_METADATA} {}

BindingKit::BindingKit(const internal::FromInputXml& fromInputXml)
    : internal::DataSetElement{"", fromInputXml, XsdType::COLLECTION_METADATA}
{}

const std::string& BindingKit::PartNumber() const { return Attribute(Element::PART_NUMBER); }

std::string& BindingKit::PartNumber() { return Attribute(Element::PART_NUMBER); }

BindingKit& BindingKit::PartNumber(std::string s)
{
    Attribute(Element::PART_NUMBER, s);
    return *this;
}

bool BindingKit::HasPartNumber() { return HasAttribute(Element::PART_NUMBER); }

// ----------------------
// Collections
// ----------------------

Collections::Collections() : internal::DataSetElement{"Collections", XsdType::NONE}
{
    Attribute("xmlns", "http://pacificbiosciences.com/PacBioCollectionMetadata.xsd");
}

Collections::Collections(const internal::FromInputXml& fromInputXml)
    : internal::DataSetElement{"", fromInputXml, XsdType::NONE}
{
    Attribute("xmlns", "http://pacificbiosciences.com/PacBioCollectionMetadata.xsd");
}

// ----------------------
// ControlKit
// ----------------------

ControlKit::ControlKit() : internal::DataSetElement{"ControlKit", XsdType::COLLECTION_METADATA} {}

ControlKit::ControlKit(const internal::FromInputXml& fromInputXml)
    : internal::DataSetElement{"", fromInputXml, XsdType::COLLECTION_METADATA}
{}

const std::string& ControlKit::PartNumber() const { return Attribute(Element::PART_NUMBER); }

std::string& ControlKit::PartNumber() { return Attribute(Element::PART_NUMBER); }

ControlKit& ControlKit::PartNumber(std::string s)
{
    Attribute(Element::PART_NUMBER, s);
    return *this;
}
bool ControlKit::HasPartNumber() const { return HasAttribute(Element::PART_NUMBER); }

const std::string& ControlKit::LeftAdapter() const
{
    if (!cache_) {
        cache_ = UpdateControlKitCache(*this);
    }
    return cache_->LeftAdapter;
}

ControlKit& ControlKit::LeftAdapter(std::string s)
{
    if (!cache_) {
        cache_ = UpdateControlKitCache(*this);
    }
    cache_->LeftAdapter = s;
    UpdateControlKit(cache_, *this);
    return *this;
}

bool ControlKit::HasLeftAdapter() const { return !LeftAdapter().empty(); }

const std::string& ControlKit::RightAdapter() const
{
    if (!cache_) {
        cache_ = UpdateControlKitCache(*this);
    }
    return cache_->RightAdapter;
}

ControlKit& ControlKit::RightAdapter(std::string s)
{
    if (!cache_) {
        cache_ = UpdateControlKitCache(*this);
    }
    cache_->RightAdapter = s;
    UpdateControlKit(cache_, *this);
    return *this;
}

bool ControlKit::HasRightAdapter() const { return !RightAdapter().empty(); }

const std::string& ControlKit::Sequence() const
{
    if (!cache_) {
        cache_ = UpdateControlKitCache(*this);
    }
    return cache_->Sequence;
}

ControlKit& ControlKit::Sequence(std::string s)
{
    if (!cache_) {
        cache_ = UpdateControlKitCache(*this);
    }
    cache_->Sequence = s;
    UpdateControlKit(cache_, *this);
    return *this;
}

bool ControlKit::HasSequence() const { return !Sequence().empty(); }

// ----------------------
// PPAConfig
// ----------------------

PPAConfig::PPAConfig() : internal::DataSetElement{"PPAConfig", XsdType::COLLECTION_METADATA} {}

PPAConfig::PPAConfig(const internal::FromInputXml& fromInputXml)
    : internal::DataSetElement{"", fromInputXml, XsdType::COLLECTION_METADATA}
{}

const std::string& PPAConfig::Json() const { return Text(); }

std::string& PPAConfig::Json() { return Text(); }

PPAConfig& PPAConfig::Json(std::string json)
{
    Text(std::move(json));
    return *this;
}

// ----------------------
// SequencingKitPlate
// ----------------------

SequencingKitPlate::SequencingKitPlate()
    : internal::DataSetElement{"SequencingKitPlate", XsdType::COLLECTION_METADATA}
{}

SequencingKitPlate::SequencingKitPlate(const internal::FromInputXml& fromInputXml)
    : internal::DataSetElement{"", fromInputXml, XsdType::COLLECTION_METADATA}
{}

const std::string& SequencingKitPlate::PartNumber() const
{
    return Attribute(Element::PART_NUMBER);
}

std::string& SequencingKitPlate::PartNumber() { return Attribute(Element::PART_NUMBER); }

SequencingKitPlate& SequencingKitPlate::PartNumber(std::string s)
{
    Attribute(Element::PART_NUMBER, s);
    return *this;
}

bool SequencingKitPlate::HasPartNumber() const { return HasAttribute(Element::PART_NUMBER); }

// ----------------------
// TemplatePrepKit
// ----------------------

TemplatePrepKit::TemplatePrepKit()
    : internal::DataSetElement{"TemplatePrepKit", XsdType::COLLECTION_METADATA}
{}

TemplatePrepKit::TemplatePrepKit(const internal::FromInputXml& fromInputXml)
    : internal::DataSetElement{"", fromInputXml, XsdType::COLLECTION_METADATA}
{}

const std::string& TemplatePrepKit::PartNumber() const { return Attribute(Element::PART_NUMBER); }

std::string& TemplatePrepKit::PartNumber() { return Attribute(Element::PART_NUMBER); }

TemplatePrepKit& TemplatePrepKit::PartNumber(std::string s)
{
    Attribute(Element::PART_NUMBER, s);
    return *this;
}

bool TemplatePrepKit::TemplatePrepKit::HasPartNumber() const
{
    return HasAttribute(Element::PART_NUMBER);
}

std::string TemplatePrepKit::LeftAdaptorSequence() const
{
    return ChildText(Element::LEFT_ADAPTOR_SEQUENCE);
}

TemplatePrepKit& TemplatePrepKit::LeftAdaptorSequence(std::string s)
{
    ChildText(Element::LEFT_ADAPTOR_SEQUENCE, s);
    return *this;
}

bool TemplatePrepKit::HasLeftAdaptorSequence() const
{
    return HasChild(Element::LEFT_ADAPTOR_SEQUENCE);
}

std::string TemplatePrepKit::LeftPrimerSequence() const
{
    return ChildText(Element::LEFT_PRIMER_SEQUENCE);
}

TemplatePrepKit& TemplatePrepKit::LeftPrimerSequence(std::string s)
{
    ChildText(Element::LEFT_PRIMER_SEQUENCE, s);
    return *this;
}

bool TemplatePrepKit::HasLeftPrimerSequence() const
{
    return HasChild(Element::LEFT_PRIMER_SEQUENCE);
}

std::string TemplatePrepKit::RightAdaptorSequence() const
{
    return ChildText(Element::RIGHT_ADAPTOR_SEQUENCE);
}

TemplatePrepKit& TemplatePrepKit::RightAdaptorSequence(std::string s)
{
    ChildText(Element::RIGHT_ADAPTOR_SEQUENCE, s);
    return *this;
}

bool TemplatePrepKit::HasRightAdaptorSequence() const
{
    return HasChild(Element::RIGHT_ADAPTOR_SEQUENCE);
}

std::string TemplatePrepKit::RightPrimerSequence() const
{
    return ChildText(Element::RIGHT_PRIMER_SEQUENCE);
}

TemplatePrepKit& TemplatePrepKit::RightPrimerSequence(std::string s)
{
    ChildText(Element::RIGHT_PRIMER_SEQUENCE, s);
    return *this;
}

bool TemplatePrepKit::HasRightPrimerSequence() const
{
    return HasChild(Element::RIGHT_PRIMER_SEQUENCE);
}

// ----------------------
// CollectionMetadata
// ----------------------

CollectionMetadata::CollectionMetadata()
    : internal::StrictEntityType{"CollectionMetadata", "CollectionMetadata",
                                 XsdType::COLLECTION_METADATA}
{}

CollectionMetadata::CollectionMetadata(const internal::FromInputXml& fromInputXml)
    : internal::StrictEntityType{"CollectionMetadata", "CollectionMetadata", fromInputXml,
                                 XsdType::COLLECTION_METADATA}
{}

CollectionMetadata::CollectionMetadata(std::string subreadSetName)
    : internal::StrictEntityType{"CollectionMetadata", "CollectionMetadata",
                                 XsdType::COLLECTION_METADATA}
    , subreadSetName_{std::move(subreadSetName)}
{}

CollectionMetadata::CollectionMetadata(std::string subreadSetName,
                                       const internal::FromInputXml& fromInputXml)
    : internal::StrictEntityType{"CollectionMetadata", "CollectionMetadata", fromInputXml,
                                 XsdType::COLLECTION_METADATA}
    , subreadSetName_{std::move(subreadSetName)}
{}

const std::string& CollectionMetadata::SubreadSetName() const { return subreadSetName_; }

DEFINE_ACCESSORS(CollectionMetadata, Automation, Automation)

CollectionMetadata& CollectionMetadata::Automation(BAM::Automation automation)
{
    Automation() = automation;
    return *this;
}

bool CollectionMetadata::HasAutomation() const { return HasChild(Element::AUTOMATION); }

const BAM::AutomationParameters& CollectionMetadata::AutomationParameters() const
{
    const BAM::Automation& automation = Automation();
    return automation.AutomationParameters();
}

BAM::AutomationParameters& CollectionMetadata::AutomationParameters()
{
    BAM::Automation& automation = Automation();
    return automation.AutomationParameters();
}

CollectionMetadata& CollectionMetadata::AutomationParameters(BAM::AutomationParameters params)
{
    // BAM::Automation& automation = Automation();
    AutomationParameters() = params;
    return *this;
}

bool CollectionMetadata::HasAutomationParameters() const
{
    return HasAutomation() && Automation().HasAutomationParameters();
}

DEFINE_ACCESSORS(CollectionMetadata, BindingKit, BindingKit)

CollectionMetadata& CollectionMetadata::BindingKit(BAM::BindingKit kit)
{
    BindingKit() = std::move(kit);
    return *this;
}

bool CollectionMetadata::HasBindingKit() const { return HasChild("BindingKit"); }

DEFINE_ACCESSORS(CollectionMetadata, ControlKit, ControlKit)

CollectionMetadata& CollectionMetadata::ControlKit(BAM::ControlKit kit)
{
    ControlKit() = std::move(kit);
    return *this;
}

bool CollectionMetadata::HasControlKit() const { return HasChild("ControlKit"); }

DEFINE_ACCESSORS(CollectionMetadata, PPAConfig, PPAConfig)

CollectionMetadata& CollectionMetadata::PPAConfig(BAM::PPAConfig config)
{
    PPAConfig() = std::move(config);
    return *this;
}

bool CollectionMetadata::HasPPAConfig() const { return HasChild("PPAConfig"); }

DEFINE_ACCESSORS(CollectionMetadata, SequencingKitPlate, SequencingKitPlate)

CollectionMetadata& CollectionMetadata::SequencingKitPlate(BAM::SequencingKitPlate kit)
{
    SequencingKitPlate() = std::move(kit);
    return *this;
}

bool CollectionMetadata::HasSequencingKitPlate() const { return HasChild("SequencingKitPlate"); }

DEFINE_ACCESSORS(CollectionMetadata, TemplatePrepKit, TemplatePrepKit)

CollectionMetadata& CollectionMetadata::TemplatePrepKit(BAM::TemplatePrepKit kit)
{
    TemplatePrepKit() = std::move(kit);
    return *this;
}

bool CollectionMetadata::HasTemplatePrepKit() const { return HasChild("TemplatePrepKit"); }

CollectionMetadata CollectionMetadata::FromRawXml(const std::string& input)
{
    // load XML
    pugi::xml_document doc;
    const pugi::xml_parse_result loadResult = doc.load_string(input.c_str());
    if (loadResult.status != pugi::status_ok) {
        throw std::runtime_error{
            "[pbbam] dataset ERROR: could not create CollectionMetadata from raw XML, error code:" +
            std::to_string(loadResult.status)};
    }
    pugi::xml_node rootNode = doc.document_element();

    // top-level attributes
    CollectionMetadata cm{internal::FromInputXml{}};
    cm.Label(rootNode.name());
    auto attributeIter = rootNode.attributes_begin();
    auto attributeEnd = rootNode.attributes_end();
    for (; attributeIter != attributeEnd; ++attributeIter) {
        std::string name = attributeIter->name();
        std::string value = attributeIter->value();
        cm.Attribute(std::move(name), std::move(value));
    }

    // iterate children, recursively building up subtree
    auto childIter = rootNode.begin();
    auto childEnd = rootNode.end();
    for (; childIter != childEnd; ++childIter) {
        pugi::xml_node childNode = *childIter;
        CollectionMetadataElementFromXml(childNode, cm);
    }

    return cm;
}

}  // namespace BAM
}  // namespace PacBio
