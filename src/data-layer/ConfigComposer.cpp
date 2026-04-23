#include "../../include/data-layer/ConfigComposer.hpp"
using namespace std;

Config ConfigComposer::buildConfig(const string &pathProp, const string &pathRail, const string &pathUtil, const string &pathTax, const string &pathAction, const string &pathSpecial, const string &pathMisc)
{
    PropertyConfig propHandler(pathProp);
    RailroadConfig railHandler(pathRail);
    UtilityConfig utilHandler(pathUtil);
    TaxConfig taxHandler(pathTax);
    ActionTileConfig actionHandler(pathAction);
    SpecialTileConfig specialHandler(pathSpecial);
    MiscTileConfig miscHandler(pathMisc);

    Config config (
        propHandler.getData(),
        railHandler.getRailRoadRentTable(),
        utilHandler.getUtilityMultiplier(),
        taxHandler.getTaxConfig(),
        specialHandler.getSpecialTileConfig(),
        miscHandler.getMiscTileConfig(),
        actionHandler.getActionTileConfig()
    );
}

Config &ConfigComposer::getConfig()
{
    return currentConfig;
}

ConfigComposer::ConfigComposer(const string& pathProp, const string& pathRail, const string& pathUtil, const string& pathTax, const string& pathAction, const string& pathSpecial, const string& pathMisc) : currentConfig(buildConfig(pathProp, pathRail, pathUtil, pathTax, pathAction, pathSpecial, pathMisc)){}