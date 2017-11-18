#include "AppxWindows.hpp"
#include "Exceptions.hpp"
#include "StreamBase.hpp"
#include "StorageObject.hpp"
#include "AppxSignature.hpp"
#include "AppxPackaging.hpp"
#include "HashStream.hpp"
#include "ComHelper.hpp"
#include "SignatureValidator.hpp"
#include "BlockMapStream.hpp"

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <set>

namespace xPlat {

AppxSignatureObject::AppxSignatureObject(APPX_VALIDATION_OPTION validationOptions, IStream* stream) : 
    VerifierObject(stream), 
    m_validationOptions(validationOptions)
{
    m_hasDigests = SignatureValidator::Validate(validationOptions, stream, m_digests, m_signatureOrigin);

    if (0 == (validationOptions & APPX_VALIDATION_OPTION::APPX_VALIDATION_OPTION_SKIPSIGNATURE))
    {   // reset the source stream back to the beginning after validating it.
        LARGE_INTEGER li{0};    
        ThrowHrIfFailed(stream->Seek(li, StreamBase::Reference::START, nullptr));
    }
}

IStream* AppxSignatureObject::GetValidationStream(const std::string& part, IStream* stream)
{
    if (m_hasDigests)
    {
        if (part == std::string("AppxBlockMap.xml"))
        {
            // This stream implementation will throw if the underlying stream does not match the digest
            auto result = ComPtr<IStream>::Make<HashStream>(stream, this->GetAppxBlockMapDigest());
            return result.Detach();
        }
        else if (part == std::string("[Content_Types].xml"))
        {
            // This stream implementation will throw if the underlying stream does not match the digest'
            auto result = ComPtr<IStream>::Make<HashStream>(stream, this->GetContentTypesDigest());
            return result.Detach();
        }
        else if (part == std::string("CodeIntegrity.cat"))
        {
            // This stream implementation will throw if the underlying stream does not match the digest
            auto result = ComPtr<IStream>::Make<HashStream>(stream, this->GetCodeIntegrityDigest());
            return result.Detach();
        }    
        // TODO: unnamed stream for central directory?
    }
    return stream;
}

} // namespace xPlat