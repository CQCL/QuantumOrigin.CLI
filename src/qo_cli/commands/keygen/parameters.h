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
#pragma once

#include "parameter_base.h"
#include <api_parameters.h>
#include <decryption_parameters.h>

#include <qo_common/key_parameters.h>
#include <qo_common/parameters.h>

#include <optional>
#include <string>
#include <unordered_map>

namespace Quantinuum::QuantumOrigin::Cli::Commands::Keygen
{

    class KeyParameters
    {
      public:
        std::string json; // Parameters read as a raw JSON
    };

    class KeygenParameters : public IParameters
    {
      public:
        ApiParameters apiParameters;
        std::optional<Common::KeyAlgorithm> keyAlgorithm;
        std::optional<Common::KeyType> keyType;
        KeyParameters keyParameters;

        DecryptionParameters decryptionParameters;
        OutputParameters outputParameters;

        void print() const override;
    };

} // namespace Quantinuum::QuantumOrigin::Cli::Commands::Keygen
