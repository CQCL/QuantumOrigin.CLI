/* Copyright 2023 Cambridge Quantum Computing Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "command.h"
#include "utils.h"

#ifdef INCLUDE_SUPPORT_FOR_KEYDECRYPT
#include "decrypt.h"
#endif

#include <qo_common/exceptions.h>
#include <qo_common/key_request.h>
#include <qo_common/service.h>

#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>

#include <random>

namespace Quantinuum::QuantumOrigin::Cli::Commands::Keygen
{
    void KeygenCommand::useConfigParameters()
    {
        const auto &config = getConfig();
        if (config)
        {
            if (getParameters().apiParameters.authParameters.clientCertificateFilename.empty())
            {
                getParameters().apiParameters.authParameters.clientCertificateFilename = config->getCredentialsConfig().getCertificate();
            }
            if (getParameters().apiParameters.authParameters.privateKeyFilename.empty())
            {
                getParameters().apiParameters.authParameters.privateKeyFilename = config->getCredentialsConfig().getPrivateKeyForCert();
            }
            if (!getParameters().keyType)
            {
                getParameters().keyType = config->getKeyParametersConfig().getKeyType();
            }
            if (!getParameters().keyAlgorithm)
            {
                getParameters().keyAlgorithm = config->getKeyParametersConfig().getKeyAlgorithm();
            }
            if (getParameters().keyParameters.json.empty())
            {
                getParameters().keyParameters.json = config->getKeyParametersConfig().getKeyParameters();
            }
            if (getParameters().apiParameters.url.empty())
            {
                getParameters().apiParameters.url = config->getCredentialsConfig().getUrl();
            }
            if (getParameters().decryptionParameters.nonce.empty())
            {
                getParameters().decryptionParameters.nonce = config->getKeyParametersConfig().getNonce();
            }
            if (!getParameters().outputParameters.outputFormat)
            {
                getParameters().outputParameters.outputFormat = config->getGeneralConfig().getOutputFormat();
            }
            if (getParameters().outputParameters.outputFilename.empty())
            {
                getParameters().outputParameters.outputFilename = config->getGeneralConfig().getOutputFilename();
            }
            if (getParameters().decryptionParameters.sharedSecret.empty())
            {
                getParameters().decryptionParameters.sharedSecret = config->getKeyParametersConfig().getSharedSecret();
            }
            if (getParameters().apiParameters.authParameters.apiKey.empty())
            {
                getParameters().apiParameters.authParameters.apiKey = config->getCredentialsConfig().getApiKey();
            }
            if (getParameters().apiParameters.authParameters.clientId.empty())
            {
                getParameters().apiParameters.authParameters.clientId = config->getCredentialsConfig().getClientId();
            }
        }
    }

    void KeygenCommand::checkParameters()
    {
        if (getParameters().apiParameters.authParameters.clientId.empty())
        {
            if (getParameters().apiParameters.authParameters.clientCertificateFilename.empty() || getParameters().apiParameters.authParameters.privateKeyFilename.empty())
            {
                throw std::runtime_error("Either a client ID or client cert/key parameters are required");
            }
        }
        if (!getParameters().keyType && !getParameters().keyAlgorithm)
        {
            throw MissingParameterError("Key type or algorithm");
        }
        if (getParameters().keyAlgorithm && getParameters().keyParameters.json.empty())
        {
            throw MissingParameterError("Key parameters");
        }
        if (getParameters().apiParameters.url.empty())
        {
            throw MissingParameterError("URL");
        }

#ifdef INCLUDE_SUPPORT_FOR_KEYDECRYPT
        if (getParameters().decryptionParameters.sharedSecret.empty())
        {
            throw MissingParameterError("Shared secret");
        }
#endif

        if (getParameters().decryptionParameters.nonce.empty())
        {
            // Generate random nonce if one wasn't supplied
            spdlog::debug("No nonce supplied, generating random nonce");

            std::vector<uint8_t> nonce(16);

            std::random_device randomDevice;
            std::independent_bits_engine<std::default_random_engine, sizeof(uint8_t), uint8_t> randomBitsEngine(randomDevice());
            std::generate(begin(nonce), end(nonce), std::ref(randomBitsEngine));

            getParameters().decryptionParameters.nonce = nonce;
        }
    }

    void KeygenCommand::execute()
    {
        spdlog::info("Working on [keygen]");
        std::unique_ptr<Common::Connection> keyGenConnection;
        if (getParameters().apiParameters.authParameters.clientId.empty())
        {
            // clientId is not present, we use the cert+key based ctor
            keyGenConnection = std::make_unique<Common::Connection>(
                getParameters().apiParameters.url, getParameters().apiParameters.authParameters.clientCertificateFilename, Common::CertType::PEM,
                getParameters().apiParameters.authParameters.privateKeyFilename, getParameters().apiParameters.authParameters.apiKey);
        }
        else
        {
            // clientId is present, we use the matching ctor.
            keyGenConnection = std::make_unique<Common::Connection>(
                getParameters().apiParameters.url, getParameters().apiParameters.authParameters.clientId, getParameters().apiParameters.authParameters.apiKey);
        }

        auto retrieveKey = [&](auto &keyRequest)
        {
            auto keyResponse = Utils::sendRequestWithRetries(*keyGenConnection, keyRequest);
#ifdef INCLUDE_SUPPORT_FOR_KEYDECRYPT
            spdlog::info("Working on [keydecrypt]");
            spdlog::debug("Encoded payload len = {}", keyResponse.encrypted.encryptedData.size());

            CliDecrypt keyDecrypt(getParameters().decryptionParameters.sharedSecret, getParameters().decryptionParameters.nonce, keyResponse);
            keyDecrypt.runDecrypt(getParameters().outputParameters.outputFormat, getParameters().outputParameters.getOutputStream());
#else
            spdlog::trace("Encrypted response: {}", spdlog::to_hex(std::begin(keyResponse.encrypted.encryptedData), std::end(keyResponse.encrypted.encryptedData)));
#endif // INCLUDE_SUPPORT_FOR_KEYDECRYPT
        };

        // Default to using -t input which overrides any -a/-p inputs

        if (getParameters().keyType)
        {
            Common::KeyRequest keyRequest(*getParameters().keyType, getParameters().decryptionParameters.nonce);
            retrieveKey(keyRequest);
        }
        else
        {
            Common::KeyRequest keyRequest(*getParameters().keyAlgorithm, getParameters().keyParameters.json, getParameters().decryptionParameters.nonce);
            retrieveKey(keyRequest);
        }
    }

} // namespace Quantinuum::QuantumOrigin::Cli::Commands::Keygen
