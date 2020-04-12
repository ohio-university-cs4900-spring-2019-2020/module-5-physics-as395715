#pragma once

#include "NetMsg.h"
#include <string>
#ifdef AFTR_CONFIG_USE_BOOST

namespace Aftr
{

    class Module4Message : public NetMsg
    {
    public:
        NetMsgMacroDeclaration(Module4Message);

        Module4Message();
        virtual ~Module4Message();
        virtual bool toStream(NetMessengerStreamBuffer& os) const;
        virtual bool fromStream(NetMessengerStreamBuffer& is);
        virtual void onMessageArrived();
        virtual std::string toString() const;

        Vector position;
        Vector size;
        std::string model;

        std::string message;

    protected:

    };
}
#endif