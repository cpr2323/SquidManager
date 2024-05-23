#pragma once

#include <JuceHeader.h>
#include "../SquidBankProperties.h"
#include "../SquidChannelProperties.h"
#include "../../Utility/RuntimeRootProperties.h"

class EditManager
{
public:
    void init (juce::ValueTree rootPropertiesVT);

    void loadBank (juce::File bankDirectory);

private:
    RuntimeRootProperties runtimeRootProperties;
    SquidBankProperties squidBankProperties;
    std::array<SquidChannelProperties, 8> squidChannelProperties;
};