// Copyright (c) 2019, Caleb Marshall.
//
// This file is part of Toontown OTP.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// You should have received a copy of the MIT License
// along with Toontown OTP. If not, see <https://opensource.org/licenses/MIT>.

#include "config_libotp.h"
#include "dconfig.h"

#include "network.h"
#include "messagedirector.h"

Configure(config_libotp);
NotifyCategoryDef(libotp , "");

ConfigureFn(config_libotp)
{
  init_libotp();
}

void init_libotp()
{
  static bool initialized = false;
  if (initialized)
  {
    return;
  }

  NetworkConnector::init_type();
  NetworkHandler::init_type();
  NetworkAcceptor::init_type();

  Participant::init_type();
  MessageDirector::init_type();
  ParticipantInterface::init_type();

  initialized = true;
}
