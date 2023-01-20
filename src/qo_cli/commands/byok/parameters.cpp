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
#include "parameters.h"

#include <magic_enum.hpp>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/spdlog.h>

namespace Quantinuum::QuantumOrigin::Cli::Commands::Byok
{

    void ByokParameters::print() const
    {
        spdlog::debug("ByokParameters");
        spdlog::debug("cert                  = \"{}\"", apiParameters.authParameters.clientCertificateFilename);
        spdlog::debug("privateKeyForCert     = \"{}\"", apiParameters.authParameters.privateKeyFilename);
        spdlog::debug("url                   = \"{}\"", apiParameters.url);
        spdlog::debug("nonce                 = \"{}\"", spdlog::to_hex(std::begin(decryptionParameters.nonce), std::end(decryptionParameters.nonce)));
        spdlog::debug("sharedSecret          = \"{}\"", spdlog::to_hex(std::begin(decryptionParameters.sharedSecret), std::end(decryptionParameters.sharedSecret)));
        spdlog::debug("ApiKey                = \"{}\"", apiParameters.authParameters.apiKey);
    }

} // namespace Quantinuum::QuantumOrigin::Cli::Commands::Byok
