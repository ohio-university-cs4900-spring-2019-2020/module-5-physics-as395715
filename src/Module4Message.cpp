#include "NetMsgCreateWO.h"
#ifdef AFTR_CONFIG_USE_BOOST

#include <iostream>
#include <sstream>
#include <string>
#include "AftrManagers.h"
#include "Vector.h"
#include "WO.h"
#include "GLView.h"
#include "WorldContainer.h"
#include "Module4Message.h"
#include "GLViewNewModule.h"
#include "Model.h"
#include "WOLight.h"

using namespace Aftr;

NetMsgMacroDefinition(Module4Message);

Module4Message::Module4Message()
{
    this->message = message;
}

Module4Message::~Module4Message()
{
}

bool Module4Message::toStream(NetMessengerStreamBuffer& os) const
{
    os << this->message;
    os << model;
    os << size.x;
    os << size.y;
    os << size.z;
    os << position.x;
    os << position.y;
    os << position.z;
    return true;
}

bool Module4Message::fromStream(NetMessengerStreamBuffer& is)
{
    is >> this->message;
    is >> model;
    is >> size.x;
    is >> size.y;
    is >> size.z;
    is >> position.x;
    is >> position.y;
    is >> position.z;
    return true;
}

void Module4Message::onMessageArrived()
{
    GLViewNewModule* glView = ((GLViewNewModule*)ManagerGLView::getGLView());

    glView->createUfo();
    //WO* wo = WO::New(model, size);
    //wo->setPosition(position);
    //ManagerGLView::getGLView()->getWorldContainer()->push_back(wo);
    //std::cout << this->toString() << std::endl;
}

std::string Module4Message::toString() const
{
    std::stringstream ss;

    ss << NetMsg::toString();
    ss << " \n  Payload: Message: " << this->message << "\n";
    ss << " \n  Payload: Model: " << model << "\n";
    ss << " \n  Payload: Size: " << size.x << ", " << size. y << ", " << size.z << "\n";
    ss << " \n  Payload: Position: " << position.x << ", " << position.y << ", " << position.z << "\n";

    return ss.str();
}
#endif