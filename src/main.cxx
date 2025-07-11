/*
 *  main.cxx
 *  Copyright 2025 AeroStun
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include <cosciev/context.hxx>
#include <cosciev/main.hxx>

namespace cosciev {

Main::~Main() noexcept { m_handle.destroy(); }

[[noreturn]] void Main::run() const { m_handle.promise().context().run(); }

} // namespace cosciev

int main(int argc, char** argv) { co_main(argc, argv).run(); }
