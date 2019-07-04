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

  NetworkHandler::init_type();
  NetworkAcceptor::init_type();

  Participant::init_type();
  MessageDirector::init_type();
  ParticipantInterface::init_type();

  initialized = true;
}
