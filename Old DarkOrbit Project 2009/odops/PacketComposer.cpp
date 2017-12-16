#include "PacketComposer.h"
using namespace Constants::Send;

template<typename T>
inline std::string PacketComposer::to_string(T t) {
	return boost::lexical_cast<std::string>(t);
}

inline std::string PacketComposer::to_string(bool b){
	return b ? "1" : "0";
}

//silly, this already is a string
inline std::string PacketComposer::to_string(std::string str)
{
	return str;
}

std::string PacketComposer::roundFloatToString(float number, size_t decimalplace) {
	//i'm going to make this method due to the wrong rounding in the client and the jackpot value being shown in a wrong way (500.20000012 jackpot dollar instead of 500.20)
	size_t sizeind = decimalplace?decimalplace:decimalplace - 1; //if there is no decimal place we will remove the . ("500." -> "500") 
	if (number < 10)// 4.00 -> 4 
		sizeind += 2;
	else if (number < 100) // 94.30 -> 5
		sizeind += 3;
	else if (number < 1000) // 250.45 -> 6
		sizeind += 4;
	else if (number < 10000) //1200.00 -> 7
		sizeind += 5;
	else // 10000.00
		sizeind += 6;	
	return to_string(number).substr(0, sizeind);
}

template<typename T>
inline void PacketComposer::push(T t) {
	i.push_back(to_string(t));
}

template<typename T, typename ... O>
inline void PacketComposer::push(T t, O... o) {
	//recursive until no args left
	i.push_back(to_string(t));
	this->push(o...);
}
template <typename T, typename ... O>
inline std::deque<std::string> PacketComposer::generateStringDeque(T t, O... other) {
	i.clear();
	this->push(t, other...);
	return i;
}
std::string PacketComposer::error_noHP()
{
	Packeter packet(ERR);
	packet.push("1");
	return packet;
}

std::string PacketComposer::error_notLoggedIn()
{
	Packeter packet(ERR);
	packet.push("2");
	return packet;
}

std::string PacketComposer::error_alreadyLoggedIn()
{
	Packeter packet(ERR);
	packet.push("3");
	return packet;
}

std::string PacketComposer::error_invalidSession()
{
	Packeter packet(ERR);
	packet.push("4");
	return packet;
}

std::string PacketComposer::move(id_t userID_, pos_t lx, pos_t ly, pos_t px, pos_t py ,speed_t speed, long long ms)
{
	Packeter packet(ALIENMOVE);
	std::deque<std::string> information = generateStringDeque(userID_, px, py);

	/* simple calculation on how to get the amount of time (ms) from x to y
		1. calculate delta by: planned x - local x (pow will make it positive)
		2. delta x + delta y = delta deque
		3. square root delta deque to get hypothenuse
		4. calculate points per second by hypothenuse 
		5. seconds * 1000 = milliseconds 
		6. weird constant at the end that works with the current client

		this is wrong because the speed should be exponentially raised, maybe try some logarithmic calculation, because the further you have to move the slower it gets
		*/
	information.push_back(to_string(ms));
	for (auto& str : information)
		packet.push(str);
	return packet;
}

std::string PacketComposer::events(bool inDemizone, bool playRepanim, bool inTradezone, bool inRadiationzone, bool inJumpzone, bool repvouchs, pos_t x, pos_t y)
{
	Packeter packet(EVENTS);
	std::deque<std::string> information = generateStringDeque(
		to_string(x), to_string(y),
		inDemizone, playRepanim, inTradezone, inRadiationzone, inJumpzone, repvouchs);

	return packet.pushAndGet(information);
}
std::string PacketComposer::events(bool inDemizone, bool playRepanim, bool inTradezone, bool inRadiationzone, bool inJumpzone, bool repvouchs)
{
	Packeter packet(EVENTS);
	std::deque<std::string> information = generateStringDeque(
		"NaN", "NaN", // x|y
		inDemizone, playRepanim, inTradezone, inRadiationzone, inJumpzone, repvouchs);

	return packet.pushAndGet(information);
}


std::string PacketComposer::laserAttack(id_t from, id_t to, lasertype_t lasercolor)
{
	Packeter packet(LASERSHOOT);
	packet.push(generateStringDeque(from, to, lasercolor));
	return packet;
}
std::string PacketComposer::laserAbort(id_t from,id_t to)
{
	Packeter packet(ABORTATTACK);
	packet.push(generateStringDeque(from, to));
	return packet;
}

std::string PacketComposer::rocketAttack(id_t from, id_t to, rockettype_t rocketType, bool hit)
{
	Packeter packet(ROCKETSHOOT);
	std::deque<std::string> information = generateStringDeque(from, to, hit,rocketType);

	return packet.pushAndGet(information);
}

std::string PacketComposer::init(id_t userID, username_t username, shipid_t shipID, speed_t speed, shield_t shield, 
	shield_t shieldmax, health_t health, health_t healthmax, cargo_t cargospace, cargo_t cargospacemax, pos_t x, pos_t y, 
	map_t mapID, factionid_t fractionid, clanid_t clanid, ammo_t battmax, rocket_t rocketsmax, weaponstate_t oState, 
	bool isPremium, exp_t experience, hon_t honour, level_t level, credits_t credits, uri_t uridium, 
	jackpot_t jackpot, rank_t userrank, clan_t clantag, gates_t gatesAchieved, bool useSysFont)
{
	//this method takes between 29000 (smallest) to 49000 (highest) nanoseconds which is pretty long but i mean its only rarely used
	Packeter packet(INITALIZE);
	std::deque<std::string> information = generateStringDeque(userID, username, shipID, speed, shield, shieldmax, health, healthmax, cargospace, cargospacemax, x, y, mapID, fractionid, clanid, battmax, rocketsmax,
		oState, isPremium, experience, honour, level, credits, uridium, roundFloatToString(jackpot,2), userrank, clantag, gatesAchieved, useSysFont);


	return packet.pushAndGet(information);
}
std::string PacketComposer::loadMap(map_t mapid) {
	Packeter packet(CHANGEMAP_I);
	return packet.pushAndGet(to_string(mapid));
}
std::string PacketComposer::loadMiniMap(map_t mapid) {
	Packeter packet(CHANGEMAP_M);
	return packet.pushAndGet(to_string(mapid));
}
std::string PacketComposer::spawnEnemy(id_t userID_, shipid_t shipID, weaponstate_t oState, clan_t clan, username_t username, pos_t x, pos_t y, 
	factionid_t company, clanid_t clanID, rank_t rank, bool showRedSquareOnMinimap, clanstate_t clanDiplo, gates_t galaxygates, bool usebigfont)
{
	Packeter packet(SPAWNOPPONENT);
	std::deque<std::string> information = generateStringDeque(userID_, shipID, oState, clan, username, x, y, company, clanID, rank, showRedSquareOnMinimap, clanDiplo, galaxygates, usebigfont);


	return packet.pushAndGet(information);
}

std::string PacketComposer::updateOres(ore_t prometium, ore_t endurium, ore_t terbium, ore_t xenomit, ore_t prometid, ore_t duranium, ore_t promerium)
{
	Packeter packet(UPDATEORES);
	std::deque<std::string> information = generateStringDeque(prometium, endurium, terbium, xenomit, prometid, duranium, promerium);
	return packet.pushAndGet(information);
}

std::string PacketComposer::buyAmmo(lasertype_t lasertype, size_t amount)
{
	//5|0|b|type|amount
	Packeter packet(BUY);
	packet.push("o");
	packet.push("b");
	return packet.pushAndGet(generateStringDeque(lasertype, amount));
}

std::string PacketComposer::buyRockets(rockettype_t rockettype, size_t amount)
{
	Packeter packet(BUY);
	packet.push("o");
	packet.push("r");
	return packet.pushAndGet(generateStringDeque(rockettype, amount));
}

std::string PacketComposer::setOrePrices()
{
	return std::string();
}

std::string PacketComposer::setRocketPrices()
{
	return std::string();
}

std::string PacketComposer::setAmmoPrices()
{
	return std::string();
}

std::string PacketComposer::removeOpponent(id_t id)
{
	Packeter packet(REMOVEOPPONENT);
	packet.push(to_string(id));
	return packet;
}

std::string PacketComposer::createPortal(portalid_t portalID, portaltype_t portalType, portalid_t ptarid, pos_t x, pos_t y)
{
	Packeter packet(SPAWNPORTAL);
	std::deque<std::string> information{
		to_string(portalID),
		to_string(portalType),
		to_string(ptarid), //UNUSED IN CLIENT (deprecated?)
		to_string(x),
		to_string(y)
	};
	
	for (auto& it : information) {
		packet.push(it);
	}
	return packet;
}

std::string PacketComposer::createStation(stationid_t stationID, stationtype_t stationType, stationname_t stationName, factionid_t faction, bool stationPeace, pos_t x, pos_t y)
{
	Packeter packet(SPAWNSTATION);
	std::deque<std::string> information = generateStringDeque(stationID, stationType, stationName, faction, stationPeace, x, y);
	packet.push(information);
	return packet;
}

std::string PacketComposer::updateSpeed(speed_t speed)
{
	Packeter packet(UPDATESPEED);
	packet.push(to_string(speed));
	return packet;
}

std::string PacketComposer::missSelf()
{
	Packeter packet(RECEIVEMISS);
	return packet;
}
std::string PacketComposer::missSelected()
{
	Packeter packet(MAKEMISS);
	return packet;
}
std::string PacketComposer::damageBubbleSelf(health_t health, shield_t shield, damage_t dmg)
{
	Packeter packet(RECEIVEDAMAGE);

	if (dmg <= 0) {
		return missSelf();
	}
	packet.push(to_string(health));
	packet.push(to_string(shield));
	packet.push("0"); // [4] is not used
	packet.push(to_string(dmg));

	return packet;
}
std::string PacketComposer::damageBubbleSelected(health_t health, shield_t shield, damage_t dmg, bool updateAmmo)
{
	Packeter packet(MAKEDAMAGE);

	if (dmg <= 0) {
		return missSelected();
	}

	if (updateAmmo) {
		packet.push("L");
	}
	else {
		packet.push("NaN");
	}
	packet.push(to_string(health));
	packet.push(to_string(shield));
	packet.push(to_string(dmg));

	return packet;
}

std::string PacketComposer::displayStarSystem()
{
	return Packeter(DISPLAYSTARSYSTEM);
}

std::string PacketComposer::updateSettings(bool boostenwtfisthis, bool displayDamage, bool displayAllLaser, bool displayExplosions, bool displayPlayerNames, bool displayCompanyIcon, bool displayAlphaBackground, bool ignoreResources, bool ignoreBoxes, bool convertGateswtfisthis, bool convertOpponentwtfisthis, bool playSound, bool playMusic, bool displayStatus, bool displayAttackbubble, bool selectedLaser, bool selectedRocket, bool displayChat, bool displayDrones, bool showStarsystem, bool ignoreCargo, bool ignoreHostileCargo, bool autochangeAmmo, bool enableFastBuy)
{
	return std::string();
}

std::string PacketComposer::showBars(health_t hp, health_t hpmax, shield_t shd, shield_t shdmax) {
	Packeter packet(SHOWHPANDSHDOPPONENT);
	packet.push("0");
	packet.push("0");
	packet.push(generateStringDeque(shd, shdmax, hp, hpmax));
	return packet;
}

std::string PacketComposer::kill(id_t id)
{
	return Packeter(KILLUSER);
}