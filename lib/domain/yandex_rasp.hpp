#pragma once

#include <array>

namespace route_finder {

inline constexpr const char* kParamApiKey = "apikey";
inline constexpr const char* kParamFrom = "from";
inline constexpr const char* kParamTo = "to";
inline constexpr const char* kParamDate = "date";
inline constexpr const char* kParamFormat = "format";
inline constexpr const char* kParamLang = "lang";
inline constexpr const char* kParamTransfers = "transfers";
inline constexpr const char* kParamTransportTypes = "transport_types";

inline constexpr const char* kTransportTrain = "train";
inline constexpr const char* kTransportSuburban = "suburban";
inline constexpr const char* kTransportBus = "bus";
inline constexpr const char* kTransportPlane = "plane";
inline constexpr const char* kTransportHelicopter = "helicopter";
inline constexpr const char* kTransportSea = "sea";
inline constexpr const char* kTransportTram = "tram";
inline constexpr const char* kTransportFunicular = "funicular";

inline constexpr std::array<const char*, 8> kKnownYandexTransportTypes = {
    kTransportTrain,      kTransportSuburban, kTransportBus,
    kTransportPlane,      kTransportHelicopter, kTransportSea,
    kTransportTram,       kTransportFunicular,
};

}
