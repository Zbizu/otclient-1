MarketProtocol = {}

-- private functions

local silent
local protocol
local statistics = runinsandbox('offerstatistic')

local function send(msg) if protocol and not silent then protocol:send(msg) end end

local function readMarketOffer(msg, action, var)
    local timestamp = msg:getU32()
    local counter = msg:getU16()

    local itemId = 0
    if var == MarketRequest.MyOffers or var == MarketRequest.MyHistory then
        itemId = msg:getU16()
    else
        itemId = var
    end

    local amount = msg:getU16()
    local price = msg:getU32()
    local playerName
    local state = MarketOfferState.Active
    if var == MarketRequest.MyHistory then
        state = msg:getU8()
    elseif var == MarketRequest.MyOffers then
    else
        playerName = msg:getString()
    end

    return MarketOffer.new({timestamp, counter}, action, Item.create(itemId),
                           amount, price, playerName, state, var)
end

-- parsing protocols
local function parseMarketEnter(protocol, msg)
    local offerCount = msg:getU8()

    local depotItems = {}
    local depotCount = msg:getU16()
    for i = 1, depotCount do
        local itemId = msg:getU16() -- item id
        local itemCount = msg:getU16() -- item count

        depotItems[itemId] = itemCount
    end

    signalcall(Market.onMarketEnter, depotItems, offerCount)
    return true
end

local function parseMarketLeave(protocol, msg)
    Market.onMarketLeave()
    return true
end

local function parseMarketDetail(protocol, msg)
    local itemId = msg:getU16()
    local descriptions = {}
    for i = MarketItemDescription.First, MarketItemDescription.Last do
        if msg:peekU16() ~= 0x00 then
            table.insert(descriptions, {i, msg:getString()}) -- item descriptions
        else
            msg:getU16()
        end
    end
    local time = (os.time() / 1000) * statistics.SECONDS_PER_DAY;

    local purchaseStats = {}
    local count = msg:getU8()
    for i = 1, count do
        local transactions = msg:getU32() -- transaction count
        local totalPrice = msg:getU32() -- total price
        local highestPrice = msg:getU32() -- highest price
        local lowestPrice = msg:getU32() -- lowest price

        local tmp = time - statistics.SECONDS_PER_DAY
        table.insert(purchaseStats,
                     OfferStatistic.new(tmp, MarketAction.Buy, transactions,
                                        totalPrice, highestPrice, lowestPrice))
    end

    local saleStats = {}
    count = msg:getU8()
    for i = 1, count do
        local transactions = msg:getU32() -- transaction count
        local totalPrice = msg:getU32() -- total price
        local highestPrice = msg:getU32() -- highest price
        local lowestPrice = msg:getU32() -- lowest price

        local tmp = time - statistics.SECONDS_PER_DAY
        table.insert(saleStats,
                     OfferStatistic.new(tmp, MarketAction.Sell, transactions,
                                        totalPrice, highestPrice, lowestPrice))
    end

    signalcall(Market.onMarketDetail, itemId, descriptions, purchaseStats,
               saleStats)
    return true
end

local function parseMarketBrowse(protocol, msg)
    local var = msg:getU16()
    local offers = {}

    local buyOfferCount = msg:getU32()
    for i = 1, buyOfferCount do
        table.insert(offers, readMarketOffer(msg, MarketAction.Buy, var))
    end

    local sellOfferCount = msg:getU32()
    for i = 1, sellOfferCount do
        table.insert(offers, readMarketOffer(msg, MarketAction.Sell, var))
    end

    signalcall(Market.onMarketBrowse, offers)
    return true
end

local function parseMarketResourcesBalance(protocol, msg)
    local resourceType = msg:getU8() -- type
    local value = msg:getU64() -- value

    signalcall(Market.onMarketResourceBalance, resourceType, value)
    return true
end

local function parseStoreBalance(protocol, msg)
	msg:getU8() -- 0x01
	msg:getU32() -- store coins (transferable + non-t)
	msg:getU32() -- transferable coints
	msg:getU32() -- reserved store coins
	msg:getU32() -- tournament coins
	return true
end

-- public functions
function initProtocol()
    connect(g_game, {
        onGameStart = MarketProtocol.registerProtocol,
        onGameEnd = MarketProtocol.unregisterProtocol
    })

    -- reloading module
    if g_game.isOnline() then MarketProtocol.registerProtocol() end

    MarketProtocol.silent(false)
end

function terminateProtocol()
    disconnect(g_game, {
        onGameStart = MarketProtocol.registerProtocol,
        onGameEnd = MarketProtocol.unregisterProtocol
    })

    -- reloading module
    MarketProtocol.unregisterProtocol()
    MarketProtocol = nil
end

function MarketProtocol.updateProtocol(_protocol) protocol = _protocol end

function MarketProtocol.registerProtocol()
    ProtocolGame.registerOpcode(GameServerOpcodes.GameServerMarketEnter,
                                parseMarketEnter)
    ProtocolGame.registerOpcode(GameServerOpcodes.GameServerMarketLeave,
                                parseMarketLeave)
    ProtocolGame.registerOpcode(GameServerOpcodes.GameServerMarketDetail,
                                parseMarketDetail)
    ProtocolGame.registerOpcode(GameServerOpcodes.GameServerMarketBrowse,
                                parseMarketBrowse)
    ProtocolGame.registerOpcode(GameServerOpcodes.GameServerSendResourceBalance,
                                parseMarketResourcesBalance)
	-- to do: move to store module
    ProtocolGame.registerOpcode(GameServerOpcodes.GameServerCoinBalance,
                                parseStoreBalance)
    MarketProtocol.updateProtocol(g_game.getProtocolGame())
end

function MarketProtocol.unregisterProtocol()
    ProtocolGame.unregisterOpcode(GameServerOpcodes.GameServerMarketEnter,
                                  parseMarketEnter)
    ProtocolGame.unregisterOpcode(GameServerOpcodes.GameServerMarketLeave,
                                  parseMarketLeave)
    ProtocolGame.unregisterOpcode(GameServerOpcodes.GameServerMarketDetail,
                                  parseMarketDetail)
    ProtocolGame.unregisterOpcode(GameServerOpcodes.GameServerMarketBrowse,
                                  parseMarketBrowse)
    ProtocolGame.unregisterOpcode(
        GameServerOpcodes.GameServerSendResourceBalance,
        parseMarketResourcesBalance)
    ProtocolGame.unregisterOpcode(GameServerOpcodes.GameServerCoinBalance,
                                  parseStoreBalance)
    MarketProtocol.updateProtocol(nil)
end

function MarketProtocol.silent(mode) silent = mode end

-- sending protocols

function MarketProtocol.sendMarketLeave()
    local msg = OutputMessage.create()
    msg:addU8(ClientOpcodes.ClientMarketLeave)
    send(msg)
end

function MarketProtocol.sendMarketBrowse(browseId)
    local msg = OutputMessage.create()
    msg:addU8(ClientOpcodes.ClientMarketBrowse)
    msg:addU16(browseId)
    send(msg)
end

function MarketProtocol.sendMarketBrowseMyOffers()
    MarketProtocol.sendMarketBrowse(MarketRequest.MyOffers)
end

function MarketProtocol.sendMarketCreateOffer(type, spriteId, amount, price,
                                              anonymous)
    local msg = OutputMessage.create()
    msg:addU8(ClientOpcodes.ClientMarketCreate)
    msg:addU8(type)
    msg:addU16(spriteId)
    msg:addU16(amount)
    msg:addU32(price)
    msg:addU8(anonymous)
    send(msg)
end

function MarketProtocol.sendMarketCancelOffer(timestamp, counter)
    local msg = OutputMessage.create()
    msg:addU8(ClientOpcodes.ClientMarketCancel)
    msg:addU32(timestamp)
    msg:addU16(counter)
    send(msg)
end

function MarketProtocol.sendMarketAcceptOffer(timestamp, counter, amount)
    local msg = OutputMessage.create()
    msg:addU8(ClientOpcodes.ClientMarketAccept)
    msg:addU32(timestamp)
    msg:addU16(counter)
    msg:addU16(amount)
    send(msg)
end