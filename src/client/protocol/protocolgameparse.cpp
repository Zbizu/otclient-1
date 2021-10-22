/*
 * Copyright (c) 2010-2020 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <client/protocol/protocolgame.h>

#include <client/thing/creature/localplayer.h>
#include <client/manager/thingtypemanager.h>
#include <client/game.h>
#include <client/map/map.h>
#include <client/thing/item.h>
#include <client/thing/effect.h>
#include <client/thing/missile.h>
#include <client/map/tile.h>
#include <client/lua/luavaluecasts.h>
#include <framework/core/eventdispatcher.h>

void ProtocolGame::parseMessage(const InputMessagePtr& msg)
{
    int16 opcode = -1;
    int16 prevOpcode = -1;

    try
    {
        while(!msg->eof())
        {
            opcode = msg->getU8();

            // try to parse in lua first
            const int readPos = msg->getReadPos();
            if(callLuaField<bool>("onOpcode", opcode, msg)) {
                continue;
            }

            msg->setReadPos(readPos); // restore read pos

            switch(opcode)
            {
            case Proto::GameServerLoginOrPendingState:
                parsePendingGame(msg);
                break;
            case Proto::GameServerGMActions:
                parseGMActions(msg);
                break;
            case Proto::GameServerUpdateNeeded:
                parseUpdateNeeded(msg);
                break;
            case Proto::GameServerLoginError:
                parseLoginError(msg);
                break;
            case Proto::GameServerLoginAdvice:
                parseLoginAdvice(msg);
                break;
            case Proto::GameServerLoginWait:
                parseLoginWait(msg);
                break;
            case Proto::GameServerLoginToken:
                parseLoginToken(msg);
                break;
            case Proto::GameServerPing:
                parsePingBack(msg);
                break;
            case Proto::GameServerPingBack:
                parsePing(msg);
                break;
            case Proto::GameServerChallenge:
                parseChallenge(msg);
                break;
            case Proto::GameServerDeath:
                parseDeath(msg);
                break;
            case Proto::GameServerOpenStash:
                parseOpenStash(msg);
                break;
            case Proto::GameServerSpecialContainersAvailable:
                parseSpecialContainersAvailable(msg);
                break;
            case Proto::GameServerFullMap:
                parseMapDescription(msg);
                break;
            case Proto::GameServerMapTopRow:
                parseMapMoveNorth(msg);
                break;
            case Proto::GameServerMapRightRow:
                parseMapMoveEast(msg);
                break;
            case Proto::GameServerMapBottomRow:
                parseMapMoveSouth(msg);
                break;
            case Proto::GameServerMapLeftRow:
                parseMapMoveWest(msg);
                break;
            case Proto::GameServerUpdateTile:
                parseUpdateTile(msg);
                break;
            case Proto::GameServerCreateOnMap:
                parseTileAddThing(msg);
                break;
            case Proto::GameServerChangeOnMap:
                parseTileTransformThing(msg);
                break;
            case Proto::GameServerDeleteOnMap:
                parseTileRemoveThing(msg);
                break;
            case Proto::GameServerMoveCreature:
                parseCreatureMove(msg);
                break;
            case Proto::GameServerOpenContainer:
                parseOpenContainer(msg);
                break;
            case Proto::GameServerCloseContainer:
                parseCloseContainer(msg);
                break;
            case Proto::GameServerCreateContainer:
                parseContainerAddItem(msg);
                break;
            case Proto::GameServerChangeInContainer:
                parseContainerUpdateItem(msg);
                break;
            case Proto::GameServerDeleteInContainer:
                parseContainerRemoveItem(msg);
                break;
            case Proto::GameServerSetInventory:
                parseAddInventoryItem(msg);
                break;
            case Proto::GameServerDeleteInventory:
                parseRemoveInventoryItem(msg);
                break;
            case Proto::GameServerOpenNpcTrade:
                parseOpenNpcTrade(msg);
                break;
            case Proto::GameServerPlayerGoods:
                parsePlayerGoods(msg);
                break;
            case Proto::GameServerCloseNpcTrade:
                parseCloseNpcTrade(msg);
                break;
            case Proto::GameServerOwnTrade:
                parseOwnTrade(msg);
                break;
            case Proto::GameServerCounterTrade:
                parseCounterTrade(msg);
                break;
            case Proto::GameServerCloseTrade:
                parseCloseTrade(msg);
                break;
            case Proto::GameServerAmbient:
                parseWorldLight(msg);
                break;
            case Proto::GameServerGraphicalEffect:
                parseMagicEffect(msg);
                break;
            case Proto::GameServerTextEffect:
                parseAnimatedText(msg);
                break;
            case Proto::GameServerMissleEffect:
                parseDistanceMissile(msg);
                break;
            case Proto::GameServerMarkCreature:
                parseCreatureMark(msg);
                break;
            case Proto::GameServerTrappers:
                parseTrappers(msg);
                break;
            case Proto::GameServerPartyState:
                parsePartyState(msg);
                break;
            case Proto::GameServerCreatureHealth:
                parseCreatureHealth(msg);
                break;
            case Proto::GameServerCreatureLight:
                parseCreatureLight(msg);
                break;
            case Proto::GameServerCreatureOutfit:
                parseCreatureOutfit(msg);
                break;
            case Proto::GameServerCreatureSpeed:
                parseCreatureSpeed(msg);
                break;
            case Proto::GameServerCreatureSkull:
                parseCreatureSkulls(msg);
                break;
            case Proto::GameServerCreatureParty:
                parseCreatureShields(msg);
                break;
            case Proto::GameServerCreatureUnpass:
                parseCreatureUnpass(msg);
                break;
            case Proto::GameServerEditText:
                parseEditText(msg);
                break;
            case Proto::GameServerEditList:
                parseEditList(msg);
                break;
                // PROTOCOL>=1038
            case Proto::GameServerPremiumTrigger:
                parsePremiumTrigger(msg);
                break;
            case Proto::GameServerPlayerData:
                parsePlayerStats(msg);
                break;
            case Proto::GameServerPlayerSkills:
                parsePlayerSkills(msg);
                break;
            case Proto::GameServerPlayerState:
                parsePlayerIcons(msg);
                break;
            case Proto::GameServerClearTarget:
                parsePlayerCancelAttack(msg);
                break;
            case Proto::GameServerPlayerModes:
                parsePlayerModes(msg);
                break;
            case Proto::GameServerTalk:
                parseTalk(msg);
                break;
            case Proto::GameServerChannels:
                parseChannelList(msg);
                break;
            case Proto::GameServerOpenChannel:
                parseOpenChannel(msg);
                break;
            case Proto::GameServerOpenPrivateChannel:
                parseOpenPrivateChannel(msg);
                break;
            case Proto::GameServerRuleViolationChannel:
                parseRuleViolationChannel(msg);
                break;
            case Proto::GameServerRuleViolationRemove:
                parseRuleViolationRemove(msg);
                break;
            case Proto::GameServerRuleViolationCancel:
                parseRuleViolationCancel(msg);
                break;
            case Proto::GameServerRuleViolationLock:
                parseRuleViolationLock(msg);
                break;
            case Proto::GameServerOpenOwnChannel:
                parseOpenOwnPrivateChannel(msg);
                break;
            case Proto::GameServerCloseChannel:
                parseCloseChannel(msg);
                break;
            case Proto::GameServerTextMessage:
                parseTextMessage(msg);
                break;
            case Proto::GameServerCancelWalk:
                parseCancelWalk(msg);
                break;
            case Proto::GameServerWalkWait:
                parseWalkWait(msg);
                break;
            case Proto::GameServerFloorChangeUp:
                parseFloorChangeUp(msg);
                break;
            case Proto::GameServerFloorChangeDown:
                parseFloorChangeDown(msg);
                break;
            case Proto::GameServerLootContainers:
                parseLootContainers(msg);
                break;
            case Proto::GameServerChooseOutfit:
                parseOpenOutfitWindow(msg);
                break;
            case Proto::GameServerVipAdd:
                parseVipAdd(msg);
                break;
            case Proto::GameServerVipState:
                parseVipState(msg);
                break;
            case Proto::GameServerVipLogout:
                parseVipLogout(msg);
                break;
            case Proto::GameServerTutorialHint:
                parseTutorialHint(msg);
                break;
            case Proto::GameServerAutomapFlag:
                parseAutomapFlag(msg);
                break;
            case Proto::GameServerQuestLog:
                parseQuestLog(msg);
                break;
            case Proto::GameServerQuestLine:
                parseQuestLine(msg);
                break;
                // PROTOCOL>=870
            case Proto::GameServerSpellDelay:
                parseSpellCooldown(msg);
                break;
            case Proto::GameServerSpellGroupDelay:
                parseSpellGroupCooldown(msg);
                break;
            case Proto::GameServerMultiUseDelay:
                parseMultiUseCooldown(msg);
                break;
                // PROTOCOL>=910
            case Proto::GameServerChannelEvent:
                parseChannelEvent(msg);
                break;
            case Proto::GameServerItemInfo:
                parseItemInfo(msg);
                break;
            case Proto::GameServerPlayerInventory:
                parsePlayerInventory(msg);
                break;
                // PROTOCOL>=950
            case Proto::GameServerPlayerDataBasic:
                parsePlayerInfo(msg);
                break;
                // PROTOCOL>=970
            case Proto::GameServerModalDialog:
                parseModalDialog(msg);
                break;
                // PROTOCOL>=980
            case Proto::GameServerLoginSuccess:
                parseLogin(msg);
                break;
            case Proto::GameServerEnterGame:
                parseEnterGame(msg);
                break;
                // PROTOCOL>=1000
            case Proto::GameServerCreatureMarks:
                parseCreaturesMark(msg);
                break;
            case Proto::GameServerCreatureType:
                parseCreatureType(msg);
                break;
                // PROTOCOL>=1055
            case Proto::GameServerBlessings:
                parseBlessings(msg);
                break;
            case Proto::GameServerUnjustifiedStats:
                parseUnjustifiedStats(msg);
                break;
            case Proto::GameServerPvpSituations:
                parsePvpSituations(msg);
                break;
            case Proto::GameServerRefreshBestiaryTracker:
                parseRefreshBestiaryTracker(msg);
                break;
            case Proto::GameServerPreset:
                parsePreset(msg);
                break;
                // PROTOCOL>=1080
            case Proto::GameServerCoinBalanceUpdating:
                parseCoinBalanceUpdating(msg);
                break;
            case Proto::GameServerCoinBalance:
                parseCoinBalance(msg);
                break;
            case Proto::GameServerRequestPurchaseData:
                parseRequestPurchaseData(msg);
                break;
            case Proto::GameServerSendShowDescription:
                parseShowDescription(msg);
                break;
            case Proto::GameServerStoreCompletePurchase:
                parseCompleteStorePurchase(msg);
                break;
            case Proto::GameServerStoreOffers:
                parseStoreOffers(msg);
                break;
            case Proto::GameServerStoreTransactionHistory:
                parseStoreTransactionHistory(msg);
                break;
            case Proto::GameServerStoreError:
                parseStoreError(msg);
                break;
            case Proto::GameServerStore:
                parseStore(msg);
                break;
                // PROTOCOL>=1097
            case Proto::GameServerStoreButtonIndicators:
                parseStoreButtonIndicators(msg);
                break;
            case Proto::GameServerSetStoreDeepLink:
                parseSetStoreDeepLink(msg);
                break;
                // otclient ONLY
            case Proto::GameServerExtendedOpcode:
                parseExtendedOpcode(msg);
                break;
            case Proto::GameServerChangeMapAwareRange:
                parseChangeMapAwareRange(msg);
                break;
                // 12.x +
            case Proto::GameServerSendClientCheck:
                parseClientCheck(msg);
                break;
            case Proto::GameServerSendGameNews:
                parseGameNews(msg);
                break;
            case Proto::GameServerSendBlessDialog:
                parseBlessDialog(msg);
                break;
            case Proto::GameServerSendRestingAreaState:
                parseRestingAreaState(msg);
                break;
            case Proto::GameServerSendUpdateImpactTracker:
                parseUpdateImpactTracker(msg);
                break;
            case Proto::GameServerSendItemsPrice:
                parseItemsPrice(msg);
                break;
            case Proto::GameServerSendUpdateSupplyTracker:
                parseUpdateSupplyTracker(msg);
                break;
            case Proto::GameServerSendUpdateLootTracker:
                parseUpdateLootTracker(msg);
                break;
            case Proto::GameServerSendKillTrackerUpdate:
                parseKillTrackerUpdate(msg);
                break;
            case Proto::GameServerSendBestiaryEntryChanged:
                parseBestiaryEntryChanged(msg);
                break;
            case Proto::GameServerSendDailyRewardCollectionState:
                parseDailyRewardCollectionState(msg);
                break;
            case Proto::GameServerSendOpenRewardWall:
                parseOpenRewardWall(msg);
                break;
            case Proto::GameServerSendDailyReward:
                parseDailyReward(msg);
                break;
            case Proto::GameServerSendRewardHistory:
                parseRewardHistory(msg);
                break;
            case Proto::GameServerSendPreyTimeLeft:
                parsePreyTimeLeft(msg);
                break;
            case Proto::GameServerSendPreyData:
                parsePreyData(msg);
                break;
            case Proto::GameServerSendPreyRerollPrice:
                parsePreyRerollPrice(msg);
                break;
            case Proto::GameServerSendImbuementWindow:
                parseImbuementWindow(msg);
                break;
            case Proto::GameServerSendCloseImbuementWindow:
                parseCloseImbuementWindow(msg);
                break;
            case Proto::GameServerSendError:
                parseError(msg);
                break;
            case Proto::GameServerSendCollectionResource:
                parseCollectionResource(msg);
                break;
            case Proto::GameServerSendTibiaTime:
                parseTibiaTime(msg);
                break;

            default:
                stdext::throw_exception(stdext::format("unhandled opcode %d", static_cast<int>(opcode)));
                break;
            }
            prevOpcode = opcode;
        }
    } catch(stdext::exception& e)
    {
        g_logger.error(stdext::format("ProtocolGame parse message exception (%d bytes unread, last opcode is 0x%02x (%d), prev opcode is 0x%02x(%d)): %s",
                                      msg->getUnreadSize(), opcode, opcode, prevOpcode, prevOpcode, e.what()));
    }
}

void ProtocolGame::parseLogin(const InputMessagePtr& msg)
{
    m_localPlayer->setId(msg->getU32());

    g_game.setServerBeat(msg->getU16());

    Creature::speedA = msg->getDouble();
    Creature::speedB = msg->getDouble();
    Creature::speedC = msg->getDouble();

    g_game.setCanReportBugs(msg->getU8());

    msg->getU8(); // can change pvp frame option
    g_game.setExpertPvpMode(msg->getU8());

    msg->getString(); // URL to ingame store images

    // premium coin package size
    // e.g you can only buy packs of 25, 50, 75, .. coins in the market
    msg->getU16();

    msg->getU8(); // Implement exiva button usage
    msg->getU8(); // Implement tournament button usage

    g_game.processLogin();
}

void ProtocolGame::parsePendingGame(const InputMessagePtr&)
{
    //set player to pending game state
    g_game.processPendingGame();
}

void ProtocolGame::parseEnterGame(const InputMessagePtr&)
{
    //set player to entered game state
    g_game.processEnterGame();

    if(!m_gameInitialized)
    {
        g_game.processGameStart();
        m_gameInitialized = true;
    }
}

void ProtocolGame::parseStoreButtonIndicators(const InputMessagePtr& msg)
{
    msg->getU8(); // have sale item
    msg->getU8(); // have new item
}

void ProtocolGame::parseSetStoreDeepLink(const InputMessagePtr& msg)
{
    msg->getU8(); // currentlyFeaturedServiceType
}

void ProtocolGame::parseBlessings(const InputMessagePtr& msg)
{
    m_localPlayer->setBlessings(msg->getU16()); // bless flag
    msg->getU8(); // TODO: add usage to blessStatus - 1 = Disabled | 2 = normal | 3 = green
}

void ProtocolGame::parsePreset(const InputMessagePtr& msg)
{
    msg->getU32(); // preset
}

void ProtocolGame::parseRequestPurchaseData(const InputMessagePtr& msg)
{
    msg->getU32(); // transactionId
    msg->getU8();  // productType
}

void ProtocolGame::parseShowDescription(const InputMessagePtr& msg)
{
    msg->getU32(); // offerId
    msg->getString();  // offer description
}

void ProtocolGame::parseStore(const InputMessagePtr& msg)
{
    parseCoinBalance(msg);

    const uint16 categories = msg->getU16();
    for(uint_fast16_t i = 0; i < categories; ++i)
    {
        msg->getString(); // category
        msg->getString(); // description

        msg->getU8(); // highlightState

        const uint8 iconCount = msg->getU8();
        std::vector<std::string> icons;
        icons.reserve(iconCount);
        for(uint_fast8_t j = 0; j < iconCount; ++j) {
            icons.push_back(msg->getString());
        }

        // If this is a valid category name then
        // the category we just parsed is a child of that
        msg->getString();
    }
}

void ProtocolGame::parseCoinBalance(const InputMessagePtr& msg)
{
    if(msg->getU8()) // update
    {
        // amount of coins that can be used to buy prodcuts
        // in the ingame store
        msg->getU32(); // coins

        // amount of coins that can be sold in market
        // or be transfered to another player
        msg->getU32(); // transferableCoins
    }
}

void ProtocolGame::parseCoinBalanceUpdating(const InputMessagePtr& msg)
{
    // coin balance can be updating and might not be accurate
    msg->getU8(); // isUpdating
}

void ProtocolGame::parseCompleteStorePurchase(const InputMessagePtr& msg)
{
    // not used
    msg->getU8();

    const std::string message = msg->getString();
    const uint32 coins = msg->getU32();
    const uint32 transferableCoins = msg->getU32();

    g_logger.info(stdext::format("Purchase Complete: %s\nAvailable coins: %d (transferable: %d)", message, coins, transferableCoins));
}

void ProtocolGame::parseStoreTransactionHistory(const InputMessagePtr& msg)
{
    msg->getU32(); // currentPage
    msg->getU32(); // pageCount

    const uint8 entries = msg->getU8();
    for(uint_fast8_t i = 0; i < entries; ++i)
    {
        const uint32 time = msg->getU32();
        const  uint8 productType = msg->getU8();
        const uint32 coinChange = msg->getU32();

        msg->getU8(); // 0 = transferable tibia coin, 1 = normal tibia coin

        const std::string productName = msg->getString();

        g_logger.error(stdext::format("Time %i, type %i, change %i, product name %s", time, productType, coinChange, productName));
    }
}

void ProtocolGame::parseStoreOffers(const InputMessagePtr& msg)
{
    msg->getString(); // categoryName

    const uint16 offers = msg->getU16();
    for(uint_fast16_t i = 0; i < offers; ++i)
    {
        msg->getU32();    // offerId
        msg->getString(); // offerName
        msg->getString(); // offerDescription

        msg->getU32(); // price

        const uint8 highlightState = msg->getU8();
        if(highlightState == 2)
        {
            msg->getU32(); // saleValidUntilTimestamp
            msg->getU32(); // basePrice
        }

        const uint8 disabledState = msg->getU8();
        if(disabledState == 1)
        {
            msg->getString(); // disabledReason
        }

        const uint8 iconCount = msg->getU8();
        std::vector<std::string> icons;
        icons.reserve(iconCount);
        for(uint_fast8_t j = 0; j < iconCount; ++j)
            icons.push_back(msg->getString());

        const uint16 subOffers = msg->getU16();
        for(uint_fast16_t j = 0; j < subOffers; ++j)
        {
            msg->getString(); // name
            msg->getString(); // description

            const uint8 subIcons = msg->getU8();
            for(uint_fast8_t k = 0; k < subIcons; ++k)
                msg->getString(); // icon

            msg->getString(); // serviceType
        }
    }
}

void ProtocolGame::parseStoreError(const InputMessagePtr& msg)
{
    const uint8 errorType = msg->getU8();
    const std::string message = msg->getString();

    g_logger.error(stdext::format("Store Error: %s [%i]", message, errorType));
}

void ProtocolGame::parseUnjustifiedStats(const InputMessagePtr& msg)
{
    UnjustifiedPoints unjustifiedPoints;
    unjustifiedPoints.killsDay = msg->getU8();
    unjustifiedPoints.killsDayRemaining = msg->getU8();
    unjustifiedPoints.killsWeek = msg->getU8();
    unjustifiedPoints.killsWeekRemaining = msg->getU8();
    unjustifiedPoints.killsMonth = msg->getU8();
    unjustifiedPoints.killsMonthRemaining = msg->getU8();
    unjustifiedPoints.skullTime = msg->getU8();

    g_game.setUnjustifiedPoints(unjustifiedPoints);
}

void ProtocolGame::parsePvpSituations(const InputMessagePtr& msg)
{
    g_game.setOpenPvpSituations(msg->getU8());
}

void ProtocolGame::parseRefreshBestiaryTracker(const InputMessagePtr& msg)
{
    const uint8 listCount = msg->getU8();
    for(uint_fast8_t i = 0; i < listCount; ++i)
    {
        msg->getU16(); // raceId
        msg->getU32(); // killAmount
        msg->getU16(); // bestiaryFirstUnlock
        msg->getU16(); // bestiarySecondUnlock
        msg->getU16(); // bestiaryToUnlock
        msg->getU8(); // ?
    }
}

void ProtocolGame::parseGMActions(const InputMessagePtr& msg)
{
    const uint8 numViolationReasons = 20;
    std::vector<uint8> actions;
    actions.reserve(numViolationReasons);
    for(uint_fast8_t i = 0; i < numViolationReasons; ++i)
        actions.push_back(msg->getU8());

    g_game.processGMActions(actions);
}

void ProtocolGame::parseUpdateNeeded(const InputMessagePtr& msg)
{
    g_game.processUpdateNeeded(msg->getString());
}

void ProtocolGame::parseLoginError(const InputMessagePtr& msg)
{
    g_game.processLoginError(msg->getString());
}

void ProtocolGame::parseLoginAdvice(const InputMessagePtr& msg)
{
    g_game.processLoginAdvice(msg->getString());
}

void ProtocolGame::parseLoginWait(const InputMessagePtr& msg)
{
    const std::string message = msg->getString();
    const uint8 time = msg->getU8();

    g_game.processLoginWait(message, time);
}

void ProtocolGame::parseLoginToken(const InputMessagePtr& msg)
{
    g_game.processLoginToken(msg->getU8() == 0);
}

void ProtocolGame::parsePing(const InputMessagePtr&)
{
    g_game.processPing();
}

void ProtocolGame::parsePingBack(const InputMessagePtr&)
{
    g_game.processPingBack();
}

void ProtocolGame::parseChallenge(const InputMessagePtr& msg)
{
    const uint32 timestamp = msg->getU32();
    const uint8 random = msg->getU8();

    sendLoginPacket(timestamp, random);
}

void ProtocolGame::parseDeath(const InputMessagePtr& msg)
{
    const uint8 deathType = msg->getU8(),
        penality = msg->getU8(),
        deathRedemption = msg->getU8();

    g_game.processDeath(deathType, penality, deathRedemption);
}

void ProtocolGame::parseOpenStash(const InputMessagePtr& msg)
{
    const uint16 size = msg->getU16();
    for(uint_fast8_t i = 0; i < size; ++i)
    {
        msg->getU16(); // id
        msg->getU32(); // count
    }

    msg->getU16(); // Stash Count
}

void ProtocolGame::parseSpecialContainersAvailable(const InputMessagePtr& msg)
{
    msg->getU8(); // isSupplyStashMenuAvailable
    msg->getU8(); // isMarketMenuAvailable
}

void ProtocolGame::parseMapDescription(const InputMessagePtr& msg)
{
    const Position pos = getPosition(msg);

    if(!m_mapKnown)
        m_localPlayer->setPosition(pos);

    g_map.setCentralPosition(pos);

    AwareRange range = g_map.getAwareRange();
    setMapDescription(msg, pos.x - range.left, pos.y - range.top, pos.z, range.horizontal(), range.vertical());

    if(!m_mapKnown)
    {
        g_dispatcher.addEvent([] { g_lua.callGlobalField("g_game", "onMapKnown"); });
        m_mapKnown = true;
    }

    g_dispatcher.addEvent([] { g_lua.callGlobalField("g_game", "onMapDescription"); });
}

void ProtocolGame::parseMapMoveNorth(const InputMessagePtr& msg)
{
    Position pos;
    if(g_game.getFeature(Otc::GameMapMovePosition))
        pos = getPosition(msg);
    else
        pos = g_map.getCentralPosition();
    --pos.y;

    g_map.setCentralPosition(pos);

    AwareRange range = g_map.getAwareRange();
    setMapDescription(msg, pos.x - range.left, pos.y - range.top, pos.z, range.horizontal(), 1);
}

void ProtocolGame::parseMapMoveEast(const InputMessagePtr& msg)
{
    Position pos;
    if(g_game.getFeature(Otc::GameMapMovePosition))
        pos = getPosition(msg);
    else
        pos = g_map.getCentralPosition();
    pos.x++;

    g_map.setCentralPosition(pos);

    AwareRange range = g_map.getAwareRange();
    setMapDescription(msg, pos.x + range.right, pos.y - range.top, pos.z, 1, range.vertical());
}

void ProtocolGame::parseMapMoveSouth(const InputMessagePtr& msg)
{
    Position pos;
    if(g_game.getFeature(Otc::GameMapMovePosition))
        pos = getPosition(msg);
    else
        pos = g_map.getCentralPosition();
    pos.y++;

    g_map.setCentralPosition(pos);

    AwareRange range = g_map.getAwareRange();
    setMapDescription(msg, pos.x - range.left, pos.y + range.bottom, pos.z, range.horizontal(), 1);
}

void ProtocolGame::parseMapMoveWest(const InputMessagePtr& msg)
{
    Position pos;
    if(g_game.getFeature(Otc::GameMapMovePosition))
        pos = getPosition(msg);
    else
        pos = g_map.getCentralPosition();
    --pos.x;

    g_map.setCentralPosition(pos);

    AwareRange range = g_map.getAwareRange();
    setMapDescription(msg, pos.x - range.left, pos.y - range.top, pos.z, 1, range.vertical());
}

void ProtocolGame::parseUpdateTile(const InputMessagePtr& msg)
{
    const Position& tilePos = getPosition(msg);
    setTileDescription(msg, tilePos);
}

void ProtocolGame::parseTileAddThing(const InputMessagePtr& msg)
{
    const Position& pos = getPosition(msg);
    const uint8 stackPos = msg->getU8();
    const auto& thing = getThing(msg);

    g_map.addThing(thing, pos, stackPos);
}

void ProtocolGame::parseTileTransformThing(const InputMessagePtr& msg)
{
    const auto& thing = getMappedThing(msg),
        newThing = getThing(msg);

    if(!thing)
    {
        g_logger.traceError("no thing");
        return;
    }

    const Position& pos = thing->getPosition();
    const uint8 stackpos = thing->getStackPos();

    if(!g_map.removeThing(thing))
    {
        g_logger.traceError("unable to remove thing");
        return;
    }

    g_map.addThing(newThing, pos, stackpos);
}

void ProtocolGame::parseTileRemoveThing(const InputMessagePtr& msg)
{
    const auto& thing = getMappedThing(msg);
    if(!thing)
    {
        g_logger.traceError("no thing");
        return;
    }

    if(!g_map.removeThing(thing))
        g_logger.traceError("unable to remove thing");
}

void ProtocolGame::parseCreatureMove(const InputMessagePtr& msg)
{
    const auto& thing = getMappedThing(msg);
    const Position& newPos = getPosition(msg);

    if(!thing || !thing->isCreature())
    {
        g_logger.traceError("no creature found to move");
        return;
    }

    if(!g_map.removeThing(thing))
    {
        g_logger.traceError("unable to remove creature");
        return;
    }

    const auto& creature = thing->static_self_cast<Creature>();
    creature->allowAppearWalk();

    g_map.addThing(thing, newPos, -1);
}

void ProtocolGame::parseOpenContainer(const InputMessagePtr& msg)
{
    const uint8 containerId = msg->getU8();
    const ItemPtr& containerItem = getItem(msg);
    const std::string name = msg->getString();
    const uint8 capacity = msg->getU8();
    const bool hasParent = (msg->getU8() != 0);

    msg->getU8(); // To-do: Depot Find (boolean)

    const bool isUnlocked = (msg->getU8() != 0); // drag and drop
    const bool hasPages = (msg->getU8() != 0);   // pagination
    const uint16 containerSize = msg->getU16(),
        firstIndex = msg->getU16(),
        itemCount = msg->getU8();

    std::vector<ItemPtr> items;
    items.reserve(itemCount);
    for(uint_fast8_t i = 0; i < itemCount; ++i)
        items.push_back(getItem(msg));

    g_game.processOpenContainer(containerId, containerItem, name, capacity, hasParent, items, isUnlocked, hasPages, containerSize, firstIndex);
}

void ProtocolGame::parseCloseContainer(const InputMessagePtr& msg)
{
    g_game.processCloseContainer(msg->getU8());
}

void ProtocolGame::parseContainerAddItem(const InputMessagePtr& msg)
{
    const uint8 containerId = msg->getU8();
    const uint16 slot = msg->getU16();
    const auto& item = getItem(msg);

    g_game.processContainerAddItem(containerId, item, slot);
}

void ProtocolGame::parseContainerUpdateItem(const InputMessagePtr& msg)
{
    const uint8 containerId = msg->getU8();
    const uint16 slot = msg->getU16();
    const auto& item = getItem(msg);

    g_game.processContainerUpdateItem(containerId, slot, item);
}

void ProtocolGame::parseContainerRemoveItem(const InputMessagePtr& msg)
{
    const uint8 containerId = msg->getU8();

    const uint16 slot = msg->getU16(),
        itemId = msg->getU16();

    ItemPtr lastItem;
    if(itemId != 0)
        lastItem = getItem(msg, itemId);

    g_game.processContainerRemoveItem(containerId, slot, lastItem);
}

void ProtocolGame::parseAddInventoryItem(const InputMessagePtr& msg)
{
    const uint8 slot = msg->getU8();
    const auto& item = getItem(msg);
    g_game.processInventoryChange(slot, item);
}

void ProtocolGame::parseRemoveInventoryItem(const InputMessagePtr& msg)
{
    g_game.processInventoryChange(msg->getU8(), ItemPtr());
}

void ProtocolGame::parseOpenNpcTrade(const InputMessagePtr& msg)
{
    msg->getString(); // NPC Name

    msg->getU16(); // Version 12.20 Feature
    msg->getString(); // Version 12.30 Feature
	
    const uint16 listCount = msg->getU16();
    std::vector<std::tuple<ItemPtr, std::string, int, int, int>> items;
    items.reserve(listCount);
    for(uint_fast16_t i = 0; i < listCount; ++i)
    {
        const auto& item = Item::create(msg->getU16());
        item->setCountOrSubType(msg->getU8());

        const std::string name = msg->getString();
        const uint32 weight = msg->getU32(),
            buyPrice = msg->getU32(),
            sellPrice = msg->getU32();

        items.emplace_back(item, name, weight, buyPrice, sellPrice);
    }

    g_game.processOpenNpcTrade(items);
}

void ProtocolGame::parsePlayerGoods(const InputMessagePtr& msg)
{
    const uint64 money = msg->getU64();

    const uint8 size = msg->getU8();
    std::vector<std::tuple<ItemPtr, uint16>> goods;
    goods.reserve(size);
    for(uint_fast8_t i = 0; i < size; ++i)
    {
        const uint16 itemId = msg->getU16();

        uint16 amount;
        if(g_game.getFeature(Otc::GameDoubleShopSellAmount))
            amount = msg->getU16();
        else
            amount = msg->getU8();

        goods.emplace_back(Item::create(itemId), amount);
    }

    g_game.processPlayerGoods(money, goods);
}

void ProtocolGame::parseCloseNpcTrade(const InputMessagePtr&)
{
    g_game.processCloseNpcTrade();
}

void ProtocolGame::parseOwnTrade(const InputMessagePtr& msg)
{
    const std::string name = g_game.formatCreatureName(msg->getString());
    const uint8 count = msg->getU8();

    std::vector<ItemPtr> items;
    items.reserve(count);
    for(uint_fast8_t i = 0; i < count; ++i)
        items.push_back(getItem(msg));

    g_game.processOwnTrade(name, items);
}

void ProtocolGame::parseCounterTrade(const InputMessagePtr& msg)
{
    const std::string name = g_game.formatCreatureName(msg->getString());
    const uint8 count = msg->getU8();

    std::vector<ItemPtr> items;
    items.reserve(count);
    for(uint_fast8_t i = 0; i < count; ++i)
        items.push_back(getItem(msg));

    g_game.processCounterTrade(name, items);
}

void ProtocolGame::parseCloseTrade(const InputMessagePtr&)
{
    g_game.processCloseTrade();
}

void ProtocolGame::parseWorldLight(const InputMessagePtr& msg)
{
    const uint8 intensity = msg->getU8();
    const uint8 color = msg->getU8();

    g_map.setLight(Light{ intensity, color });
}

void ProtocolGame::parseMagicEffect(const InputMessagePtr& msg)
{
    const Position& pos = getPosition(msg);

    auto effectType = static_cast<Otc::MagicEffectsType_t>(msg->getU8());
    while(effectType != Otc::MAGIC_EFFECTS_END_LOOP) {
        if(effectType == Otc::MAGIC_EFFECTS_DELTA) {
            msg->getU8();
        } else if(effectType == Otc::MAGIC_EFFECTS_DELAY) {
            msg->getU8(); // ?
        } else if(effectType == Otc::MAGIC_EFFECTS_CREATE_DISTANCEEFFECT) {
            const uint8_t shotId = msg->getU8();

            const int8_t offsetX = static_cast<int8_t>(msg->getU8()),
                offsetY = static_cast<int8_t>(msg->getU8());

            if(!g_things.isValidDatId(shotId, ThingCategoryMissile)) {
                g_logger.traceError(stdext::format("invalid missile id %d", shotId));
                return;
            }

            const auto& missile = MissilePtr(new Missile());
            missile->setId(shotId);
            missile->setPath(pos, Position(pos.x + offsetX, pos.y + offsetY, pos.z));

            g_map.addThing(missile, pos);
        } else if(effectType == Otc::MAGIC_EFFECTS_CREATE_DISTANCEEFFECT_REVERSED) {
            const uint8_t shotId = msg->getU8();

            const int8_t offsetX = static_cast<int8_t>(msg->getU8()),
                offsetY = static_cast<int8_t>(msg->getU8());

            if(!g_things.isValidDatId(shotId, ThingCategoryMissile)) {
                g_logger.traceError(stdext::format("invalid missile id %d", shotId));
                return;
            }

            const auto& missile = MissilePtr(new Missile());
            missile->setId(shotId);
            missile->setPath(Position(pos.x + offsetX, pos.y + offsetY, pos.z), pos);

            g_map.addThing(missile, pos);
        } else if(effectType == Otc::MAGIC_EFFECTS_CREATE_EFFECT) {
            const uint8_t effectId = msg->getU8();
            if(!g_things.isValidDatId(effectId, ThingCategoryEffect)) {
                g_logger.traceError(stdext::format("invalid effect id %d", effectId));
                continue;
            }

            const auto& effect = EffectPtr(new Effect());
            effect->setId(effectId);
            g_map.addThing(effect, pos);
        }

        effectType = static_cast<Otc::MagicEffectsType_t>(msg->getU8());
    }
}

void ProtocolGame::parseAnimatedText(const InputMessagePtr& msg)
{
    const Position& position = getPosition(msg);
    const auto& animatedText = AnimatedTextPtr(new AnimatedText);

    animatedText->setColor(msg->getU8());
    animatedText->setText(msg->getString());
    g_map.addThing(animatedText, position);
}

void ProtocolGame::parseDistanceMissile(const InputMessagePtr& msg)
{
    const auto& fromPos = getPosition(msg);
    const auto& toPos = getPosition(msg);
    const uint8 shotId = msg->getU8();

    if(!g_things.isValidDatId(shotId, ThingCategoryMissile))
    {
        g_logger.traceError(stdext::format("invalid missile id %d", shotId));
        return;
    }

    const auto& missile = MissilePtr(new Missile());
    missile->setId(shotId);
    missile->setPath(fromPos, toPos);
    g_map.addThing(missile, fromPos);
}

void ProtocolGame::parseCreatureMark(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();
    const uint8 color = msg->getU8();

    const auto& creature = g_map.getCreatureById(id);
    if(!creature) {
        g_logger.traceError("could not get creature");
        return;
    }

    creature->addTimedSquare(color);
}

void ProtocolGame::parseTrappers(const InputMessagePtr& msg)
{
    const uint8 numTrappers = msg->getU8();

    if(numTrappers > 8)
        g_logger.traceError("too many trappers");

    for(uint_fast8_t i = 0; i < numTrappers; ++i)
    {
        const auto& creature = g_map.getCreatureById(msg->getU32());
        if(!creature) {
            g_logger.traceError("could not get creature");
        }

        // TODO: set creature as trapper
    }
}

void ProtocolGame::parsePartyState(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();
    const auto stateType = (Otc::PartyState_t)msg->getU8();

    const auto& creature = g_map.getCreatureById(id);
    if(!creature) {
        g_logger.traceError("could not get creature");
    }

    if(stateType == Otc::PARTY_STATE_CREATURE_UPDATE) {
        getCreature(msg, id); // Update Creature
        return;
    }

    const uint8 value = msg->getU8();
    if(stateType == Otc::PARTY_STATE_PLAYER_MANA) {
        const auto& localPlayer = creature->static_self_cast<LocalPlayer>();
        localPlayer->setMana((localPlayer->getMaxMana() * value) / 100, localPlayer->getMaxMana());
    } else if(stateType == Otc::PARTY_STATE_CREATURE_SHOW_STATUS) {
        // Add Status
    } else if(stateType == Otc::PARTY_STATE_PLAYER_VOCATION) {
        creature->static_self_cast<LocalPlayer>()->setVocation(value);
    } else if(stateType == Otc::PARTY_STATE_CREATURE_ICON) {
        if(value) {
            msg->getU8();
            msg->getU8();
            msg->getU16();
        }
    }
}

void ProtocolGame::parseCreatureHealth(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();
    const uint8 healthPercent = msg->getU8();

    const auto& creature = g_map.getCreatureById(id);
    if(creature)
        creature->setHealthPercent(healthPercent);
}

void ProtocolGame::parseCreatureLight(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();

    Light light;
    light.intensity = msg->getU8();
    light.color = msg->getU8();

    const auto& creature = g_map.getCreatureById(id);
    if(!creature) {
        g_logger.traceError("could not get creature");
        return;
    }

    creature->setLight(light);
}

void ProtocolGame::parseCreatureOutfit(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();
    const Outfit& outfit = getOutfit(msg);

    const auto& creature = g_map.getCreatureById(id);
    if(!creature) {
        g_logger.traceError("could not get creature");
    }

    creature->setOutfit(outfit);
}

void ProtocolGame::parseCreatureSpeed(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();
    const uint16 baseSpeed = msg->getU16(),
        speed = msg->getU16();

    const auto& creature = g_map.getCreatureById(id);
    if(creature)
    {
        creature->setSpeed(speed);
        if(baseSpeed != -1)
            creature->setBaseSpeed(baseSpeed);
    }
}

void ProtocolGame::parseCreatureSkulls(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();
    const uint8 skull = msg->getU8();

    const auto& creature = g_map.getCreatureById(id);
    if(!creature) {
        g_logger.traceError("could not get creature");
        return;
    }

    creature->setSkull(skull);
}

void ProtocolGame::parseCreatureShields(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();
    const uint8 shield = msg->getU8();

    const auto& creature = g_map.getCreatureById(id);
    if(!creature) {
        g_logger.traceError("could not get creature");
    }

    creature->setShield(shield);
}

void ProtocolGame::parseCreatureUnpass(const InputMessagePtr& msg)
{
    const uint id = msg->getU32();
    const bool unpass = msg->getU8();

    const auto& creature = g_map.getCreatureById(id);
    if(!creature) {
        g_logger.traceError("could not get creature");
    }

    creature->setPassable(!unpass);
}

void ProtocolGame::parseEditText(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();

    const auto& item = getItem(msg);
    const uint32 itemId = item->getId();

    const uint16 maxLength = msg->getU16();

    const std::string text = msg->getString(),
        writer = msg->getString();

    msg->getU8(); // Show (Traded)

    const std::string date = msg->getString();

    g_game.processEditText(id, itemId, maxLength, text, writer, date);
}

void ProtocolGame::parseEditList(const InputMessagePtr& msg)
{
    const uint8 doorId = msg->getU8();
    const uint32 id = msg->getU32();
    const std::string& text = msg->getString();

    g_game.processEditList(id, doorId, text);
}

void ProtocolGame::parsePremiumTrigger(const InputMessagePtr& msg)
{
    const uint8 triggerCount = msg->getU8();

    std::vector<uint8> triggers;
    triggers.reserve(triggerCount);
    for(uint_fast8_t i = 0; i < triggerCount; ++i)
        triggers.push_back(msg->getU8());
}

void ProtocolGame::parsePlayerInfo(const InputMessagePtr& msg)
{
    const bool isPremium = msg->getU8();
    const uint32 premiumExpiration = msg->getU32();

    m_localPlayer->setPremium(isPremium, premiumExpiration);
    m_localPlayer->setVocation(msg->getU8());
    m_localPlayer->setOpenPreyWindow(msg->getU8());

    const uint16 spellCount = msg->getU16();
    std::vector<uint8> spells;
    spells.reserve(spellCount);
    for(uint_fast16_t i = 0; i < spellCount; ++i)
        spells.push_back(msg->getU8()); // spell id

    m_localPlayer->setSpells(spells);
    m_localPlayer->setMagicShield(msg->getU8());
}

void ProtocolGame::parsePlayerStats(const InputMessagePtr& msg)
{
    const uint16 health = msg->getU16(),
        maxHealth = msg->getU16();

    const uint32 freeCapacity = msg->getU32() / 100.0;
    const uint64 experience = msg->getU64();
    const uint16 level = msg->getU16();
    const uint8 levelPercent = msg->getU8();

    msg->getU16();    // base xp gain rate
    msg->getU16();    // low level bonus
    msg->getU16();    // xp boost
    msg->getU16();    // stamina multiplier (100 = 1.0x)

    const uint16 mana = msg->getU16(),
        maxMana = msg->getU16(),
        soul = msg->getU8(),
        stamina = msg->getU16(),
        baseSpeed = msg->getU16(),
        regeneration = msg->getU16(),
        training = msg->getU16();

    msg->getU16(); // xp boost time (seconds)
    msg->getU8(); // enables exp boost in the store

    msg->getU16(); // remaining mana shield
    msg->getU16(); // total mana shield

    m_localPlayer->setHealth(health, maxHealth);
    m_localPlayer->setFreeCapacity(freeCapacity);

    m_localPlayer->setExperience(experience);
    m_localPlayer->setLevel(level, levelPercent);
    m_localPlayer->setMana(mana, maxMana);

    m_localPlayer->setStamina(stamina);
    m_localPlayer->setSoul(soul);
    m_localPlayer->setBaseSpeed(baseSpeed);
    m_localPlayer->setRegenerationTime(regeneration);
    m_localPlayer->setOfflineTrainingTime(training);
}

void ProtocolGame::parsePlayerSkills(const InputMessagePtr& msg)
{
    const uint16_t magicLevel = msg->getU16(),
        baseMagicLevel = msg->getU16(),
        magicLevelPercent = msg->getU16();

    m_localPlayer->setMagicLevel(magicLevel, magicLevelPercent);
    m_localPlayer->setBaseMagicLevel(baseMagicLevel);

    for(uint8_t skill = Otc::SKILL_FIRST; skill <= Otc::SKILL_FISHING; ++skill) {
        const uint16_t level = msg->getU16(),
            baseLevel = msg->getU16(),
            levelPercent = msg->getU16();

        m_localPlayer->setSkill(static_cast<Otc::skills_t>(skill), level, levelPercent);
        m_localPlayer->setBaseSkill(static_cast<Otc::skills_t>(skill), baseLevel);
    }

    for(uint8_t i = Otc::SKILL_CRITICAL_HIT_CHANCE; i <= Otc::SKILL_LAST; ++i)
    {
        m_localPlayer->setSkill(static_cast<Otc::skills_t>(i), msg->getU16(), 0);
        m_localPlayer->setBaseSkill(static_cast<Otc::skills_t>(i), msg->getU16());
    }

    // used for imbuement (Feather)
    m_localPlayer->setTotalCapacity(msg->getU32());
    msg->getU32(); // total base capacity
}

void ProtocolGame::parsePlayerIcons(const InputMessagePtr& msg)
{
    m_localPlayer->setIcons(msg->getU32());
}

void ProtocolGame::parsePlayerCancelAttack(const InputMessagePtr& msg)
{
    const uint      seq = msg->getU32();

    g_game.processAttackCancel(seq);
}

void ProtocolGame::parsePlayerModes(const InputMessagePtr& msg)
{
    const uint8 fightMode = msg->getU8(),
        chaseMode = msg->getU8();

    const bool safeMode = msg->getU8();
    const uint8 pvpMode = msg->getU8();

    g_game.processPlayerModes(static_cast<Otc::FightModes_t>(fightMode), static_cast<Otc::ChaseModes_t>(chaseMode), safeMode, static_cast<Otc::PVPModes_t>(pvpMode));
}

void ProtocolGame::parseSpellCooldown(const InputMessagePtr& msg)
{
    const uint8 spellId = msg->getU8();
    const uint32 delay = msg->getU32();

    // TODO: verify if there are icons for spells id 170+ and remove the ternary check (if id >170 => 150)
    g_lua.callGlobalField("g_game", "onSpellCooldown", (spellId >= 170) ? 150 : spellId, delay);
}

void ProtocolGame::parseSpellGroupCooldown(const InputMessagePtr& msg)
{
    const uint8 groupId = msg->getU8();
    const uint32 delay = msg->getU32();

    g_lua.callGlobalField("g_game", "onSpellGroupCooldown", groupId, delay);
}

void ProtocolGame::parseMultiUseCooldown(const InputMessagePtr& msg)
{
    const uint32 delay = msg->getU32();

    g_lua.callGlobalField("g_game", "onMultiUseCooldown", delay);
}

void ProtocolGame::parseTalk(const InputMessagePtr& msg)
{
    msg->getU32(); // channel statement guid

    const std::string name = g_game.formatCreatureName(msg->getString());

    msg->getU8(); // Show (Traded)

    const uint16 level = msg->getU16();

    const auto messageType = static_cast<Otc::MessageMode_t>(msg->getU8());

    uint16 channelId = 0;
    Position pos;
    switch(messageType)
    {
    case Otc::MESSAGE_SAY:
    case Otc::MESSAGE_WHISPER:
    case Otc::MESSAGE_YELL:
    case Otc::MESSAGE_PRIVATE_PN:
    case Otc::MESSAGE_MONSTER_SAY:
    case Otc::MESSAGE_MONSTER_YELL:
    case Otc::MESSAGE_SPELL_USE:
    case Otc::MESSAGE_PRIVATE_NP:
        pos = getPosition(msg);
        break;

    case Otc::MESSAGE_CHANNEL_Y:
    case Otc::MESSAGE_CHANNEL_MANAGER:
    case Otc::MESSAGE_CHANNEL_O:
    case Otc::MESSAGE_CHANNEL_R1:
        channelId = msg->getU16();
        break;

    case Otc::MESSAGE_NPC_UNKOWN:
    case Otc::MESSAGE_BROADCAST:
    case Otc::MESSAGE_PRIVATE_FROM:
    case Otc::MESSAGE_PRIVATE_RED_FROM:
        break;

    default:
        stdext::throw_exception(stdext::format("unknown message type %d", messageType));
        break;
    }

    const std::string text = msg->getString();
    g_game.processTalk(name, level, messageType, text, channelId, pos);
}

void ProtocolGame::parseChannelList(const InputMessagePtr& msg)
{
    const uint8 count = msg->getU8();

    std::vector<std::tuple<uint8, std::string>> channelList;
    channelList.reserve(count);
    for(uint_fast8_t i = 0; i < count; ++i)
    {
        uint16 id = msg->getU16();
        std::string name = msg->getString();
        channelList.emplace_back(id, name);
    }

    g_game.processChannelList(channelList);
}

void ProtocolGame::parseOpenChannel(const InputMessagePtr& msg)
{
    const uint16 channelId = msg->getU16();
    const std::string name = msg->getString();

    const uint16 joinedPlayers = msg->getU16();
    for(uint_fast16_t i = 0; i < joinedPlayers; ++i) {
        g_game.formatCreatureName(msg->getString()); // player name
    }

    const uint16 invitedPlayers = msg->getU16();
    for(uint_fast16_t i = 0; i < invitedPlayers; ++i) {
        g_game.formatCreatureName(msg->getString()); // player name
    }

    g_game.processOpenChannel(channelId, name);
}

void ProtocolGame::parseOpenPrivateChannel(const InputMessagePtr& msg)
{
    const std::string name = g_game.formatCreatureName(msg->getString());
    g_game.processOpenPrivateChannel(name);
}

void ProtocolGame::parseOpenOwnPrivateChannel(const InputMessagePtr& msg)
{
    const uint16 channelId = msg->getU16();
    const std::string name = msg->getString();

    g_game.processOpenOwnPrivateChannel(channelId, name);
}

void ProtocolGame::parseCloseChannel(const InputMessagePtr& msg)
{
    const uint16 channelId = msg->getU16();

    g_game.processCloseChannel(channelId);
}

void ProtocolGame::parseRuleViolationChannel(const InputMessagePtr& msg)
{
    const uint16 channelId = msg->getU16();

    g_game.processRuleViolationChannel(channelId);
}

void ProtocolGame::parseRuleViolationRemove(const InputMessagePtr& msg)
{
    const std::string name = msg->getString();

    g_game.processRuleViolationRemove(name);
}

void ProtocolGame::parseRuleViolationCancel(const InputMessagePtr& msg)
{
    const std::string name = msg->getString();

    g_game.processRuleViolationCancel(name);
}

void ProtocolGame::parseRuleViolationLock(const InputMessagePtr& /*msg*/)
{
    g_game.processRuleViolationLock();
}

void ProtocolGame::parseTextMessage(const InputMessagePtr& msg)
{
    const auto type = static_cast <Otc::MessageMode_t> (msg->getU8());

    switch(type) {
    case Otc::MESSAGE_DAMAGE_DEALT:
    case Otc::MESSAGE_DAMAGE_RECEIVED:
    case Otc::MESSAGE_DAMAGE_OTHERS:
    {
        const Position pos = getPosition(msg);
        uint value[2];
        int color[2];

        // physical damage
        value[0] = msg->getU32();
        color[0] = msg->getU8();

        // magic damage
        value[1] = msg->getU32();
        color[1] = msg->getU8();

        for(uint_fast8_t i = 0; i < 2; ++i) {
            if(value[i] == 0)
                continue;
            auto animatedText = AnimatedTextPtr(new AnimatedText);
            animatedText->setColor(color[i]);
            animatedText->setText(stdext::to_string(value[i]));
            g_map.addThing(animatedText, pos);
        }
        break;
    }

    case Otc::MESSAGE_HEALED:
    case Otc::MESSAGE_HEALED_OTHERS:
    case Otc::MESSAGE_EXPERIENCE:
    case Otc::MESSAGE_EXPERIENCE_OTHERS:
    {
        const Position pos = getPosition(msg);
        const uint value = msg->getU32();
        const int color = msg->getU8();

        auto animatedText = AnimatedTextPtr(new AnimatedText);
        animatedText->setColor(color);
        animatedText->setText(stdext::to_string(value));
        g_map.addThing(animatedText, pos);
        break;
    }

    case Otc::MESSAGE_GUILD:
    case Otc::MESSAGE_PARTY_MANAGEMENT:
    case Otc::MESSAGE_PARTY:
        msg->getU16(); // channelId
        break;
    }

    g_game.processTextMessage(type, msg->getString());
}

void ProtocolGame::parseCancelWalk(const InputMessagePtr& msg)
{
    const auto direction = static_cast<Otc::Direction_t>(msg->getU8());

    g_game.processWalkCancel(direction);
}

void ProtocolGame::parseWalkWait(const InputMessagePtr& msg)
{
    const uint16 millis = msg->getU16();
    m_localPlayer->lockWalk(millis);
}

void ProtocolGame::parseFloorChangeUp(const InputMessagePtr& msg)
{
    Position pos;
    if(g_game.getFeature(Otc::GameMapMovePosition))
        pos = getPosition(msg);
    else
        pos = g_map.getCentralPosition();
    AwareRange range = g_map.getAwareRange();
    --pos.z;

    int skip = 0;
    if(pos.z == SEA_FLOOR)
        for(int_fast8_t i = SEA_FLOOR - AWARE_UNDEGROUND_FLOOR_RANGE; i >= 0; --i)
            skip = setFloorDescription(msg, pos.x - range.left, pos.y - range.top, i, range.horizontal(), range.vertical(), 8 - i, skip);
    else if(pos.z > SEA_FLOOR)
        skip = setFloorDescription(msg, pos.x - range.left, pos.y - range.top, pos.z - AWARE_UNDEGROUND_FLOOR_RANGE, range.horizontal(), range.vertical(), 3, skip);

    pos.x++;
    pos.y++;
    g_map.setCentralPosition(pos);
}

void ProtocolGame::parseFloorChangeDown(const InputMessagePtr& msg)
{
    Position pos;
    if(g_game.getFeature(Otc::GameMapMovePosition))
        pos = getPosition(msg);
    else
        pos = g_map.getCentralPosition();
    AwareRange range = g_map.getAwareRange();
    pos.z++;

    int skip = 0;
    if(pos.z == UNDERGROUND_FLOOR)
    {
        int_fast8_t j, i;
        for(i = pos.z, j = -1; i <= pos.z + AWARE_UNDEGROUND_FLOOR_RANGE; ++i, --j)
            skip = setFloorDescription(msg, pos.x - range.left, pos.y - range.top, i, range.horizontal(), range.vertical(), j, skip);
    } else if(pos.z > UNDERGROUND_FLOOR && pos.z < MAX_Z - 1)
        skip = setFloorDescription(msg, pos.x - range.left, pos.y - range.top, pos.z + AWARE_UNDEGROUND_FLOOR_RANGE, range.horizontal(), range.vertical(), -3, skip);

    --pos.x;
    --pos.y;
    g_map.setCentralPosition(pos);
}

void ProtocolGame::parseLootContainers(const InputMessagePtr& msg)
{
    msg->getU8(); // quickLootFallbackToMainContainer ? 1 : 0

    const uint8 containers = msg->getU8();
    for(uint_fast8_t i = 0; i < containers; ++i) {
        msg->getU8(); // id?
        msg->getU16();
    }
}

void ProtocolGame::parseOpenOutfitWindow(const InputMessagePtr& msg)
{
    Outfit currentOutfit = getOutfit(msg, true, true);
    currentOutfit.setFamiliarId(msg->getU16());

    uint16_t listSize = msg->getU16();
    std::vector<std::tuple<uint16, std::string, uint8>> outfitList;
    outfitList.reserve(listSize);
    for(uint_fast16_t i = 0; i < listSize; ++i)
    {
        const uint16 id = msg->getU16();
        const std::string name = msg->getString();
        const uint8 addons = msg->getU8();

        // TODO: identify extra null byte usage 12.x
        msg->getU8();

        outfitList.emplace_back(id, name, addons);
    }

    listSize = msg->getU16();
    std::vector<std::tuple<uint16, std::string>> mountList;
    mountList.reserve(listSize);
    for(uint_fast16_t i = 0; i < listSize; ++i)
    {
        const uint16 id = msg->getU16();
        const std::string name = msg->getString();

        // TODO: identify extra null byte usage 12.x
        msg->getU8();

        mountList.emplace_back(id, name);
    }

    listSize = msg->getU16();
    std::vector<std::tuple<uint16, std::string>> familiarList;
    familiarList.reserve(listSize);
    for(uint_fast16_t i = 0; i < listSize; ++i)
    {
        const uint16 id = msg->getU16();
        const std::string name = msg->getString();

        // TODO: identify extra null byte usage 12.x
        msg->getU8();

        familiarList.emplace_back(id, name);
    }

    // TODO: identify extra null byte usage 12.x
    msg->getU8();
    msg->getU8();

    g_game.processOpenOutfitWindow(currentOutfit, outfitList, mountList, familiarList);
}

void ProtocolGame::parseVipAdd(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();
    const std::string name = g_game.formatCreatureName(msg->getString());
    const std::string desc = msg->getString();
    const uint32 iconId = msg->getU32();
    const bool notifyLogin = msg->getU8();
    const uint8 status = msg->getU8();

    // TODO: implement vipGroups usage
    msg->getU8(); // vipGroup by id

    g_game.processVipAdd(id, name, status, desc, iconId, notifyLogin);
}

void ProtocolGame::parseVipState(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();
    const uint8 state = msg->getU8();
    g_game.processVipStateChange(id, state);
}

void ProtocolGame::parseVipLogout(const InputMessagePtr& msg)
{
    g_game.processVipStateChange(msg->getU32(), 0);
}

void ProtocolGame::parseTutorialHint(const InputMessagePtr& msg)
{
    g_game.processTutorialHint(msg->getU8());
}

void ProtocolGame::parseAutomapFlag(const InputMessagePtr& msg)
{
    msg->skipBytes(1); // TODO: understand and apply 12.x extra unknown byte

    const Position& pos = getPosition(msg);
    const int8 icon = msg->getU8();
    const std::string description = msg->getString();
    const bool remove = msg->getU8() != 0;

    if(!remove)
        g_game.processAddAutomapFlag(pos, icon, description);
    else
        g_game.processRemoveAutomapFlag(pos, icon, description);
}

void ProtocolGame::parseQuestLog(const InputMessagePtr& msg)
{
    const uint16 questsCount = msg->getU16();
    std::vector<std::tuple<uint16, std::string, bool>> questList;
    questList.reserve(questsCount);
    for(uint_fast16_t i = 0; i < questsCount; ++i)
    {
        const uint16 id = msg->getU16();
        const std::string name = msg->getString();
        const bool completed = msg->getU8();

        questList.emplace_back(id, name, completed);
    }

    g_game.processQuestLog(questList);
}

void ProtocolGame::parseQuestLine(const InputMessagePtr& msg)
{
    const uint16 questId = msg->getU16();
    const uint8 missionCount = msg->getU8();

    std::vector<std::tuple<std::string, std::string>> questMissions;
    questMissions.reserve(missionCount);
    for(uint_fast8_t i = 0; i < missionCount; ++i)
    {
        msg->getU16(); // repeated quest ID
        const std::string missionName = msg->getString(),
            missionDescrition = msg->getString();

        questMissions.emplace_back(missionName, missionDescrition);
    }

    g_game.processQuestLine(questId, questMissions);
}

void ProtocolGame::parseChannelEvent(const InputMessagePtr& msg)
{
    const uint16 channelId = msg->getU16();
    const std::string name = g_game.formatCreatureName(msg->getString());
    const uint8 type = msg->getU8();

    g_lua.callGlobalField("g_game", "onChannelEvent", channelId, name, type);
}

void ProtocolGame::parseItemInfo(const InputMessagePtr& msg)
{
    const uint8 size = msg->getU8();
    std::vector<std::tuple<ItemPtr, std::string>> list;
    list.reserve(size);
    for(uint_fast8_t i = 0; i < size; ++i)
    {
        const auto& item(new Item);
        item->setId(msg->getU16());
        item->setCountOrSubType(msg->getU8());

        const std::string desc = msg->getString();
        list.emplace_back(item, desc);
    }

    g_lua.callGlobalField("g_game", "onItemInfo", list);
}

void ProtocolGame::parsePlayerInventory(const InputMessagePtr& msg)
{
    const uint16 size = msg->getU16();
    for(uint_fast16_t i = 0; i < size; ++i)
    {
        msg->getU16(); // id
        msg->getU8();  // subtype
        msg->getU16(); // count
    }
}

void ProtocolGame::parseModalDialog(const InputMessagePtr& msg)
{
    const uint32 windowId = msg->getU32();
    const std::string title = msg->getString(),
        message = msg->getString();

    const uint8 sizeButtons = msg->getU8();
    std::vector<std::tuple<int, std::string>> buttonList;
    buttonList.reserve(sizeButtons);
    for(uint_fast8_t i = 0; i < sizeButtons; ++i)
    {
        std::string value = msg->getString();
        int buttonId = msg->getU8();
        buttonList.emplace_back(buttonId, value);
    }

    const uint8 sizeChoices = msg->getU8();
    std::vector<std::tuple<int, std::string>> choiceList;
    choiceList.reserve(sizeChoices);
    for(uint_fast8_t i = 0; i < sizeChoices; ++i)
    {
        const std::string value = msg->getString();
        const uint8 choideId = msg->getU8();
        choiceList.emplace_back(choideId, value);
    }

    const uint8 enterButton = msg->getU8(),
        escapeButton = msg->getU8();

    const bool priority = msg->getU8() == 0x01;

    g_game.processModalDialog(windowId, title, message, buttonList, enterButton, escapeButton, choiceList, priority);
}

void ProtocolGame::parseExtendedOpcode(const InputMessagePtr& msg)
{
    const uint8 opcode = msg->getU8();
    const std::string buffer = msg->getString();

    if(opcode == 0)
        m_enableSendExtendedOpcode = true;
    else if(opcode == 2)
        parsePingBack(msg);
    else
        callLuaField("onExtendedOpcode", opcode, buffer);
}

void ProtocolGame::parseChangeMapAwareRange(const InputMessagePtr& msg)
{
    const uint8 xrange = msg->getU8(),
        yrange = msg->getU8();

    AwareRange range;
    range.left = xrange / 2 - ((xrange + 1) % 2);
    range.right = xrange / 2;
    range.top = yrange / 2 - ((yrange + 1) % 2);
    range.bottom = yrange / 2;

    g_map.setAwareRange(range);
    g_lua.callGlobalField("g_game", "onMapChangeAwareRange", xrange, yrange);
}

void ProtocolGame::parseCreaturesMark(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();
    const bool isPermanent = msg->getU8() != 1;
    const uint8 markType = msg->getU8();

    const auto& creature = g_map.getCreatureById(id);
    if(!creature) {
        g_logger.traceError("could not get creature");
        return;
    }

    if(isPermanent)
    {
        if(markType == 0xff)
            creature->hideStaticSquare();
        else
            creature->showStaticSquare(Color::from8bit(markType));
    } else
        creature->addTimedSquare(markType);
}

void ProtocolGame::parseCreatureType(const InputMessagePtr& msg)
{
    const uint32 id = msg->getU32();
    const uint8 type = msg->getU8();

    if(type == Proto::CREATURETYPE_SUMMONPLAYER)
    {
        // TODO: implement player summon type specific features
        msg->getU32(); // master id
    }

    const auto& creature = g_map.getCreatureById(id);
    if(!creature) {
        g_logger.traceError("could not get creature");
        return;
    }

    creature->setType(type);
}

void ProtocolGame::setMapDescription(const InputMessagePtr& msg, int x, int y, int z, int width, int height)
{
    int startz, endz, zstep;

    if(z > SEA_FLOOR) {
        startz = z - AWARE_UNDEGROUND_FLOOR_RANGE;
        endz = std::min<int>(z + AWARE_UNDEGROUND_FLOOR_RANGE, MAX_Z);
        zstep = 1;
    } else {
        startz = SEA_FLOOR;
        endz = 0;
        zstep = -1;
    }

    int skip = 0;
    for(int_fast8_t nz = startz; nz != endz + zstep; nz += zstep)
        skip = setFloorDescription(msg, x, y, nz, width, height, z - nz, skip);
}

int ProtocolGame::setFloorDescription(const InputMessagePtr& msg, int x, int y, int z, int width, int height, int offset, int skip)
{
    for(uint_fast8_t nx = 0; nx < width; ++nx)
    {
        for(uint_fast8_t ny = 0; ny < height; ++ny)
        {
            Position tilePos(x + nx + offset, y + ny + offset, z);
            if(skip == 0)
                skip = setTileDescription(msg, tilePos);
            else
            {
                g_map.cleanTile(tilePos);
                --skip;
            }
        }
    }
    return skip;
}

int ProtocolGame::setTileDescription(const InputMessagePtr& msg, const Position& position)
{
    g_map.cleanTile(position);

    if(msg->peekU16() >= 0xff00)
        return msg->getU16() & 0xff;

    for(uint_fast8_t stackPos = 0; stackPos <= UINT8_MAX; ++stackPos)
    {
        if(msg->peekU16() >= 0xff00)
            return msg->getU16() & 0xff;

        if(stackPos > 10)
            g_logger.traceError(stdext::format("too many things, pos=%s, stackpos=%d", stdext::to_string(position), stackPos));

        const auto& thing = getThing(msg);
        g_map.addThing(thing, position, stackPos);
    }

    return 0;
}
Outfit ProtocolGame::getOutfit(const InputMessagePtr& msg, const bool addMount, const bool forceMountData)
{
    Outfit outfit;
    uint16_t lookType = msg->getU16();
    if(lookType != 0)
    {
        outfit.setCategory(ThingCategoryCreature);
        const uint8_t head = msg->getU8(),
            body = msg->getU8(),
            legs = msg->getU8(),
            feet = msg->getU8(),
            addons = msg->getU8();

        if(!g_things.isValidDatId(lookType, ThingCategoryCreature))
        {
            g_logger.traceError(stdext::format("invalid outfit looktype %d", lookType));
            lookType = 0;
        }

        outfit.getClothes().id = lookType;
        outfit.getClothes().setHead(head);
        outfit.getClothes().setBody(body);
        outfit.getClothes().setLegs(legs);
        outfit.getClothes().setFeet(feet);
        outfit.setAddons(addons);
    } else
    {
        lookType = msg->getU16();
        if(lookType == 0)
        {
            outfit.setCategory(ThingCategoryEffect);
            outfit.setAuxId(13); // invisible effect id
        } else
        {
            if(!g_things.isValidDatId(lookType, ThingCategoryItem))
            {
                g_logger.traceError(stdext::format("invalid outfit looktypeex %d", lookType));
                lookType = 0;
            }

            outfit.setCategory(ThingCategoryItem);
            outfit.setAuxId(lookType);
        }
    }

    if(addMount) {
        outfit.getMountClothes().id = msg->getU16();
        if(outfit.hasMount() || forceMountData) {
            outfit.getMountClothes().setHead(msg->getU8());
            outfit.getMountClothes().setBody(msg->getU8());
            outfit.getMountClothes().setLegs(msg->getU8());
            outfit.getMountClothes().setFeet(msg->getU8());
        }
    }

    return outfit;
}

ThingPtr ProtocolGame::getThing(const InputMessagePtr& msg)
{
    const uint16 id = msg->getU16();

    ThingPtr thing;
    if(id == 0)
        stdext::throw_exception("invalid thing id");
    else if(id == Proto::UnknownCreature || id == Proto::OutdatedCreature || id == Proto::Creature)
        thing = getCreature(msg, id);
    else if(id == Proto::StaticText) // otclient only
        thing = getStaticText(msg, id);
    else // item
        thing = getItem(msg, id);

    return thing;
}

ThingPtr ProtocolGame::getMappedThing(const InputMessagePtr& msg)
{
    const uint16 x = msg->getU16();

    ThingPtr thing;
    if(x != 0xffff)
    {
        Position pos;
        pos.x = x;
        pos.y = msg->getU16();
        pos.z = msg->getU8();

        const uint8 stackpos = msg->getU8();
        assert(stackpos != 255);

        thing = g_map.getThing(pos, stackpos);
        if(!thing)
            g_logger.traceError(stdext::format("no thing at pos:%s, stackpos:%d", stdext::to_string(pos), stackpos));
    } else
    {
        const uint32 id = msg->getU32();

        thing = g_map.getCreatureById(id);
        if(!thing)
            g_logger.traceError(stdext::format("no creature with id %u", id));
    }

    return thing;
}

CreaturePtr ProtocolGame::getCreature(const InputMessagePtr& msg, uint16 type)
{
    if(type == 0)
        type = msg->getU16();

    CreaturePtr creature;
    const bool known = (type != Proto::UnknownCreature);
    if(type == Proto::OutdatedCreature || type == Proto::UnknownCreature) {
        if(known) {
            creature = g_map.getCreatureById(msg->getU32());
            if(!creature)
                g_logger.traceError("server said that a creature is known, but it's not");
        } else {
            const uint32 removeId = msg->getU32(),
                id = msg->getU32();

            if(id == removeId) {
                creature = g_map.getCreatureById(id);
            } else {
                g_map.removeCreatureById(removeId);
            }

            const uint8 creatureType = msg->getU8();
            if(creatureType == Proto::CREATURETYPE_SUMMONPLAYER) {
                // TODO: Implement player summon creature type specific features
                msg->getU32(); // master id
            }

            const std::string name = g_game.formatCreatureName(msg->getString());

            if(creature) {
                creature->setName(name);
            } else {
                if(id == m_localPlayer->getId()) {
                    creature = m_localPlayer;
                } else if(creatureType == Proto::CREATURETYPE_PLAYER) {
                    // fixes a bug server side bug where GameInit is not sent and local player id is unknown
                    if(m_localPlayer->getId() == 0 && name == m_localPlayer->getName())
                        creature = m_localPlayer;
                    else
                        creature = PlayerPtr(new Player);
                } else if(creatureType == Proto::CREATURETYPE_MONSTER || creatureType == Proto::CREATURETYPE_SUMMONPLAYER) {
                    creature = MonsterPtr(new Monster);
                } else if(creatureType == Proto::CREATURETYPE_NPC) {
                    creature = NpcPtr(new Npc);
                } else {
                    g_logger.traceError("creature type is invalid");
                }

                if(creature) {
                    creature->setId(id);
                    creature->setName(name);

                    g_map.addCreature(creature);
                }
            }
        }

        const uint8 healthPercent = msg->getU8();
        const auto direction = static_cast<Otc::Direction_t>(msg->getU8());

        const Outfit& outfit = getOutfit(msg);

        Light light;
        light.intensity = msg->getU8();
        light.color = msg->getU8();

        const uint16 speed = msg->getU16();
        const bool iconType = msg->getU8();
        if(iconType) {
            msg->getU8();
            msg->getU8();
            msg->getU16();
        }

        const uint8 skull = msg->getU8(),
            shield = msg->getU8(),
            emblem = !known ? msg->getU8() : -1, // emblem is sent only when the creature is not known
            creatureType = msg->getU8();

        if(creatureType == Proto::CREATURETYPE_SUMMONPLAYER) {
            msg->getU32(); // master id
        } else if(creatureType == Proto::CREATURETYPE_PLAYER) {
            msg->getU8(); // vocation id
        }

        msg->getU8(); // speechBubble
        const uint8 mark = msg->getU8();
        msg->getU8(); // inspection type
        const bool unpass = msg->getU8();

        if(creature) {
            if(mark == 0xff) {
                creature->hideStaticSquare();
            } else {
                creature->showStaticSquare(Color::from8bit(mark));
            }

            creature->setHealthPercent(healthPercent);
            creature->setDirection(direction);
            creature->setOutfit(outfit);
            creature->setSpeed(speed);
            creature->setSkull(skull);
            creature->setShield(shield);
            creature->setPassable(!unpass);
            creature->setLight(light);

            if(emblem != -1) {
                creature->setEmblem(emblem);
            }

            if(creatureType != -1) {
                creature->setType(creatureType);
            }

            if(creature == m_localPlayer && !m_localPlayer->isKnown()) {
                m_localPlayer->setKnown(true);
            }
        }
    } else if(type == Proto::Creature) {
        creature = g_map.getCreatureById(msg->getU32());

        if(!creature) {
            g_logger.traceError("invalid creature");
        }

        const auto direction = static_cast<Otc::Direction_t>(msg->getU8());
        const bool unpass = msg->getU8();
        if(creature) {
            creature->turn(direction);
            creature->setPassable(!unpass);
        }
    } else {
        stdext::throw_exception("invalid creature opcode");
    }

    return creature;
}

ItemPtr ProtocolGame::getItem(const InputMessagePtr& msg, uint16 id)
{
    if(id == 0)
        id = msg->getU16();

    const auto& item = Item::create(id);
    if(item->getId() == 0)
        stdext::throw_exception(stdext::format("unable to create item with invalid id %d", id));

    if(item->isStackable() || item->isSplash() || item->isFluidContainer() || item->isChargeable())
        item->setCountOrSubType(msg->getU8());
    else if(item->isContainer()) {
        /*uint8 hasQuickLootFlags = msg->getU8();
        if(hasQuickLootFlags)
            msg->getU32(); // quick loot flags

        uint8 hasQuiverAmmoCount = msg->getU8();
        if(hasQuiverAmmoCount)
            msg->getU32(); // ammoTotal*/
    }

    // Impl Podium
    // Todo: Temporary correction, the client dat does not contain information saying if the item is podium
    if(id == 35973 || id == 35974) {
        const uint16 lookType = msg->getU16();
        if(lookType != 0) {
            msg->getU8(); // lookHead
            msg->getU8(); // lookBody
            msg->getU8(); // lookLegs
            msg->getU8(); // lookFeet
            msg->getU8(); // lookAddons
        }

        const uint16 lookMount = msg->getU16();
        if(lookMount != 0) {
            msg->getU8(); // lookHead
            msg->getU8(); // lookBody
            msg->getU8(); // lookLegs
            msg->getU8(); // lookFeet
        }

        msg->getU8(); // lookDirection
        msg->getU8(); // podiumVisible
    }

    return item;
}

StaticTextPtr ProtocolGame::getStaticText(const InputMessagePtr& msg, uint16 /*id*/)
{
    const uint8 colorByte = msg->getU8();
    const Color color = Color::from8bit(colorByte);
    const std::string fontName = msg->getString(),
        text = msg->getString();

    const auto& staticText = StaticTextPtr(new StaticText);
    staticText->setText(text);
    staticText->setFont(fontName);
    staticText->setColor(color);

    return staticText;
}

Position ProtocolGame::getPosition(const InputMessagePtr& msg)
{
    const uint16 x = msg->getU16(),
        y = msg->getU16();

    const uint8 z = msg->getU8();

    return Position(x, y, z);
}

// 12.x +
void ProtocolGame::parseClientCheck(const InputMessagePtr& msg)
{
    msg->getU32(); // 1
    msg->getU8(); // 1
}

void ProtocolGame::parseGameNews(const InputMessagePtr& msg)
{
    msg->getU32(); // 1
    msg->getU8(); // 1

    // TODO: implement game news usage
}

void ProtocolGame::parseBlessDialog(const InputMessagePtr& msg)
{
    // parse bless amount
    const uint8_t totalBless = msg->getU8(); // total bless

    // parse each bless
    for(uint_fast8_t i = 0; i < totalBless; ++i) {
        msg->getU16(); // bless bit wise
        msg->getU8(); // player bless count
        msg->getU8(); // store?
    }

    // parse general info
    msg->getU8(); // premium
    msg->getU8(); // promotion
    msg->getU8(); // pvp min xp loss
    msg->getU8(); // pvp max xp loss
    msg->getU8(); // pve exp loss
    msg->getU8(); // equip pvp loss
    msg->getU8(); // equip pve loss
    msg->getU8(); // skull
    msg->getU8(); // aol

    // parse log
    const uint8_t logCount = msg->getU8(); // log count
    for(uint_fast8_t i = 0; i < logCount; ++i) {
        msg->getU32(); // timestamp
        msg->getU8(); // color message (0 = white loss, 1 = red)
        msg->getString(); // history message
    }

    // TODO: implement bless dialog usage
}

void ProtocolGame::parseRestingAreaState(const InputMessagePtr& msg)
{
    msg->getU8(); // zone
    msg->getU8(); // state
    msg->getString(); // message

    // TODO: implement resting area state usage
}

void ProtocolGame::parseUpdateImpactTracker(const InputMessagePtr& msg)
{
    const auto type = static_cast<Otc::ImpactAnalyzerAndTracker_t>(msg->getU8());
    msg->getU32(); // amount

    if(type != Otc::ANALYZER_HEAL) {
        msg->getU8(); // CipbiaElement
        if(type == Otc::ANALYZER_DAMAGE_RECEIVED) {
            msg->getString(); // target
        }
    }
}

void ProtocolGame::parseItemsPrice(const InputMessagePtr& msg)
{
    const uint16_t priceCount = msg->getU16(); // count

    for(uint_fast16_t i = 0; i < priceCount; ++i) {
        msg->getU16(); // item client id
        msg->getU32(); // price
    }

    // TODO: implement items price usage
}

void ProtocolGame::parseUpdateSupplyTracker(const InputMessagePtr& msg)
{
    msg->getU16(); // item client ID

    // TODO: implement supply tracker usage
}

void ProtocolGame::parseUpdateLootTracker(const InputMessagePtr& msg)
{
    getItem(msg); // item
    msg->getString(); // item name

    // TODO: implement loot tracker usage
}

void ProtocolGame::parseKillTrackerUpdate(const InputMessagePtr& msg)
{
    msg->getString(); // creature name

    msg->getU16(); // creature looktype
    msg->getU8(); // head
    msg->getU8(); // body
    msg->getU8(); // legs
    msg->getU8(); // feet
    msg->getU8(); // addons

    const uint8_t corpseSize = msg->getU8(); // corpse size

    for(uint_fast8_t i = 0; i < corpseSize; ++i) {
        getItem(msg); // corpse item
    }

    // TODO: implement kill tracker usage
}

void ProtocolGame::parseBestiaryEntryChanged(const InputMessagePtr& msg)
{
    msg->getU16(); // monster ID

    // TODO: implement bestiary entry changed usage
}

void ProtocolGame::parseDailyRewardCollectionState(const InputMessagePtr& msg)
{
    msg->getU8(); // state

    // TODO: implement daily reward collection state usage
}

void ProtocolGame::parseOpenRewardWall(const InputMessagePtr& msg)
{
    msg->getU8(); // bonus shrine (1) or instant bonus (0)
    msg->getU32(); // next reward time
    msg->getU8(); // day streak day
    const uint8_t wasDailyRewardTaken = msg->getU8(); // taken (player already took reward?)

    if(wasDailyRewardTaken) {
        msg->getString(); // error message
    }

    msg->getU32(); // time left to pickup reward without loosing streak
    msg->getU16(); // day streak level
    msg->getU16(); // unknown

    // TODO: implement open reward wall usage
}

void ProtocolGame::parseDailyReward(const InputMessagePtr& msg)
{
    msg->getU8(); // state

    // TODO: implement daily reward usage
}

void ProtocolGame::parseRewardHistory(const InputMessagePtr& msg)
{
    const uint8_t historyCount = msg->getU8(); // history count

    for(uint_fast8_t i = 0; i < historyCount; ++i) {
        msg->getU32(); // timestamp
        msg->getU8(); // is Premium
        msg->getString(); // description
        msg->getU16(); // daystreak
    }

    // TODO: implement reward history usage
}
void ProtocolGame::parsePreyTimeLeft(const InputMessagePtr& /*msg*/)
{
    // TODO: implement protocol parse
}

void ProtocolGame::getPreyMonster(const InputMessagePtr& msg)
{
    msg->getString(); // mosnter name
    msg->getU16(); // looktype
    msg->getU8(); // head
    msg->getU8(); // body
    msg->getU8(); // legs
    msg->getU8(); // feet
    msg->getU8(); // addons
}

void ProtocolGame::getPreyMonsters(const InputMessagePtr& msg)
{
    const uint8_t monstersSize = msg->getU8(); // monster list size
    for(uint8_t i = 0; i < monstersSize; ++i)
        getPreyMonster(msg);
}

void ProtocolGame::parsePreyData(const InputMessagePtr& msg)
{
    msg->getU8(); // slot
    msg->getU8(); // slot state

    // if is initialization, get empty stuff
   /* if(msg->peekU8() == 0x00) {
        msg->getU8();
    } else {
        switch(slotState) {
        case Otc::PREY_STATE_LOCKED:
            msg->getU8(); // prey slot unlocked
            break;
        case Otc::PREY_STATE_INACTIVE:
            break;
        case Otc::PREY_STATE_ACTIVE:
            getPreyMonster(msg);
            msg->getU8(); // bonus type
            msg->getU16(); // bonus value
            msg->getU8(); // bonus grade
            msg->getU16(); // time left
            break;
        case Otc::PREY_STATE_SELECTION:
            getPreyMonsters(msg);
            break;
        case Otc::PREY_STATE_SELECTION_CHANGE_MONSTER:
            msg->getU8(); // bonus type
            msg->getU16(); // bonus value
            msg->getU8(); // bonus grade
            getPreyMonsters(msg);
            break;
        }
    }*/

    msg->getU8(); // empty byte
    msg->getU32(); // next free roll
    msg->getU8(); // wildcards

    // TODO: implement prey data usage
}

void ProtocolGame::parsePreyRerollPrice(const InputMessagePtr& msg)
{
    msg->getU32(); // reroll price
    msg->getU8(); // wildcard
    msg->getU8(); // selectCreatureDirectly price (5 in tibia)

    // Prey Task
    msg->getU32();
    msg->getU32();
    msg->getU8();
    msg->getU8();
}

void ProtocolGame::getImbuementInfo(const InputMessagePtr& msg)
{
    msg->getU32(); // imbuid
    msg->getString(); // name
    msg->getString(); // description
    msg->getString(); // subgroup

    msg->getU16(); // iconId
    msg->getU32(); // duration

    msg->getU8(); // is premium

    const uint8_t itemsSize = msg->getU8(); // items size
    for(uint8_t i = 0; i < itemsSize; ++i) {
        msg->getU16(); // item client ID
        msg->getString(); // item name
        msg->getU16(); // count
    }

    msg->getU32(); // base price
    msg->getU8(); // base percent
    msg->getU32(); // base protection
}

void ProtocolGame::parseImbuementWindow(const InputMessagePtr& msg)
{
    msg->getU16(); // item client ID

    const uint8_t slot = msg->getU8(); // slot id
    for(uint8_t i = 0; i < slot; ++i) {
        const uint8_t firstByte = msg->getU8();
        if(firstByte == 0x01) {
            getImbuementInfo(msg);
            msg->getU32(); // info >> 8
            msg->getU32(); // removecust
        }
    }

    const uint16_t imbSize = msg->getU16(); // imbuement size
    for(uint_fast16_t i = 0; i < imbSize; ++i) {
        getImbuementInfo(msg);
    }

    const uint32_t neededItemsSize = msg->getU32(); // needed items size
    for(uint_fast32_t i = 0; i < neededItemsSize; ++i) {
        msg->getU16(); // item client id
        msg->getU16(); // item count
    }

    // TODO: implement imbuement window usage
}

void ProtocolGame::parseCloseImbuementWindow(const InputMessagePtr& /*msg*/)
{
    // TODO: implement close imbuement window usage
}

void ProtocolGame::parseError(const InputMessagePtr& msg)
{
    msg->getU8(); // error code
    msg->getString(); // error

    // TODO: implement error usage
}

void ProtocolGame::parseCollectionResource(const InputMessagePtr& msg)
{
    msg->getU8(); // id (0x1 bank, 0x2 inventory, 0xA prey)
    msg->getU64(); // resource value

    // TODO: implement collection resource usage
}

void ProtocolGame::parseTibiaTime(const InputMessagePtr& msg)
{
    msg->getU8(); // hours
    msg->getU8(); // minutes

    // TODO: implement tibia time usage
}
