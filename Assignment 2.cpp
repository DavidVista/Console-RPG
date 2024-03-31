#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <memory>

// Output stream

#define sysout game->getOutput()

// Exceptions

class CharacterDoesNotOwnItem : public std::exception {
public:
	const char* what() {
		return "Character doesn't exist";
	}
};

class CharacterDoesNotExist : public std::exception {
public:
	const char* what() {
		return "Character doesn't own an item";
	}
};

class IllegalHealthValue : public std::exception {
public:
	const char* what() {
		return "Negative value or zero for a potion healValue";
	}
};

class IllegalDamageValue : public std::exception {
public:
	const char* what() {
		return "Negative value or zero for a weapon's damageValue";
	}
};

class NotAllowedTarget : public std::exception {
public:
	const char* what() {
		return "Target is not in the list of allowed targets in casting spells";
	}
};

class FullContainer : public std::exception {
public:
	const char* what() {
		return "Arsenal, MedicalBag, or SpellBook are full";
	}
};

class IllegalItemType : public std::exception {
public:
	const char* what() {
		return "Character can't carry or use a certain item";
	}
};

class ElementNotFound : public std::exception {
public:
	const char* what() {
		return "No such element in a container";
	}
};

// Forward declarations of General Classes

class Character;
class PhysicalItem;
class Weapon;
class Potion;
class Spell;
class WeaponUser;
class PotionUser;
class SpellUser;
class Fighter;
class Archer;
class Wizard;
class Game;

// Concepts

template<typename T>
concept DerivedFromPhysicalItem = std::derived_from<T, PhysicalItem>;

template<typename T>
concept Printable =
	requires (std::ostream & os, T x) { x.print(os); };

template<typename T>
concept Comparable =
	requires (T x, T y) { x > y; x < y; };

template<typename T>
concept ComparableAndPritable = Printable<T> && Comparable<T>;

// Classes

template<typename T>
class Container {
private:
	std::vector<std::shared_ptr<T>> elements;
public:

	Container() : elements() {}

	virtual ~Container() {
		for (auto& itemPtr : elements) {
			itemPtr.reset();
		}
	}

	int size() const {
		return elements.size();
	}

	bool find(const std::shared_ptr<const T> item) const {
		for (auto& element : elements) {
			if (element == item) {
				return true;
			}
		}

		return false;
	}

	void removeItem(const std::shared_ptr<const T> newItem) {
		for (int i = 0; i < elements.size(); ++i) {
			if (elements[i] == newItem) {
				elements.erase(elements.begin() + i);
				return;
			}
		}

		throw ElementNotFound();
	}

	virtual void addItem(std::shared_ptr<T> newItem) {
		elements.push_back(newItem);
	}

	std::vector<std::shared_ptr<T>> getElements() const {
		return elements;
	}
};

template<DerivedFromPhysicalItem T>
class Container<T> {
private:
	std::unordered_map<std::string, std::shared_ptr<T>> map;
public:

	Container();

	virtual ~Container();

	int size() const;

	virtual void addItem(std::shared_ptr<T> newItem);

	void removeItem(const std::string& itemName);

	bool find(const std::string& itemName) const;

	std::shared_ptr<T> get(const std::string& itemName) const;

	std::vector<std::shared_ptr<T>> getElements() const;

};

template<ComparableAndPritable T>
class ContainerWithMaxCapacity : public Container<T> {
private:
	int maxCapacity;
public:
	ContainerWithMaxCapacity(int maxCapacity);

	~ContainerWithMaxCapacity();

	void addItem(std::shared_ptr<T> newItem) override;

	void show() const;
};

class Character : public std::enable_shared_from_this<Character> {
private:
	std::string name;
	int healthPoints;

	void takeDamage(int damage);

	void heal(int healValue);
protected:
	virtual void obtainItem(std::shared_ptr<PhysicalItem> item) = 0;
	virtual void loseItem(const std::shared_ptr<PhysicalItem> item) = 0;
	virtual void setup() const = 0;
public:
	friend class PhysicalItem;
	friend class Game;

	Character(const std::string nameString, int healthValue);

	virtual ~Character();

	std::string getName() const;

	int getHp() const;

	bool operator > (const Character& other) const;

	bool operator < (const Character& other) const;

	virtual void print(std::ostream& out) const = 0;
};

class Game {
private:
	static std::shared_ptr<Game> game;
	Container<Character> characters;
	std::ifstream input;
	std::ofstream output;

	std::shared_ptr<Character> getCharacterByName(std::string name) const;

	void showCharacters();

	Game();
public:

	~Game();

	void startNewGame();

	static std::shared_ptr<Game> currentGame();

	void destroyCharacter(std::shared_ptr<Character> ptr);

	std::ofstream& getOutput();
};

// Container for Physical Items Methods

template<DerivedFromPhysicalItem T>
Container<T>::Container() : map() {}

template<DerivedFromPhysicalItem T>
Container<T>::~Container() {
	for (auto it = map.begin(); it != map.end(); ++it) {
		auto& itemPtr = it->second;
		itemPtr.reset();
	}
}

template<DerivedFromPhysicalItem T>
int Container<T>::size() const {
	return map.size();
}

template<DerivedFromPhysicalItem T>
void Container<T>::addItem(std::shared_ptr<T> newItem) {
	map.insert(std::make_pair(newItem->getName(), newItem));
}

template<DerivedFromPhysicalItem T>
void Container<T>::removeItem(const std::string& itemName) {
	if (map.erase(itemName) == 0) {
		throw ElementNotFound();
	}
}

template<DerivedFromPhysicalItem T>
bool Container<T>::find(const std::string& itemName) const {
	return map.contains(itemName);
}

template<DerivedFromPhysicalItem T>
std::shared_ptr<T> Container<T>::get(const std::string& itemName) const {
	auto result = map.find(itemName);
	if (result == map.end()) {
		throw ElementNotFound();
	}
	return result->second;
}

template<DerivedFromPhysicalItem T>
std::vector<std::shared_ptr<T>> Container<T>::getElements() const {
	std::vector<std::shared_ptr<T>> result(map.size());
	int i = 0;
	for (auto it = map.begin(); it != map.end(); ++it) {
		result[i] = it->second;
	}
	return result;
}

// Container with Max Capacity Methods

template<ComparableAndPritable T>
ContainerWithMaxCapacity<T>::ContainerWithMaxCapacity(int maxCapacity) : Container<T>(), maxCapacity(maxCapacity) {}

template<ComparableAndPritable T>
ContainerWithMaxCapacity<T>::~ContainerWithMaxCapacity() = default;

template<ComparableAndPritable T>
void ContainerWithMaxCapacity<T>::addItem(std::shared_ptr<T> newItem) {
	if (this->size() == maxCapacity) {
		throw FullContainer();
	}
	this->Container<T>::addItem(newItem);
}

template<ComparableAndPritable T>
void ContainerWithMaxCapacity<T>::show() const {
	auto v = this->getElements();
	auto game = Game::currentGame();
	std::sort(v.begin(), v.end(), [](const std::shared_ptr<T> first, const std::shared_ptr<T> second) {
		return (*first < *second);
	});
	for (auto& element : v) {
		element->print(sysout);
	}
	sysout << std::endl;
}

// Character Methods

void Character::takeDamage(int damage) {
	healthPoints -= damage;
	if (healthPoints <= 0) {
		auto game = Game::currentGame();
		game->destroyCharacter(this->shared_from_this());
	}
}

void Character::heal(int healValue) {
	healthPoints += healValue;
}

Character::Character(const std::string nameString, int healthValue) : name(nameString), healthPoints(healthValue) {}

Character::~Character() {
	auto game = Game::currentGame();
	sysout << name << " has died...\n";
};

std::string	Character::getName() const {
	return name;
}

int Character::getHp() const {
	return healthPoints;
}

bool Character::operator > (const Character& other) const {
	return (name.compare(other.getName()) > 0);
}

bool Character::operator < (const Character& other) const {
	return (name.compare(other.getName()) < 0);
}

class PhysicalItem : public std::enable_shared_from_this<PhysicalItem> {
private:
	bool isUsableOnce;
	std::string name;
protected:
	std::shared_ptr<Character> owner;

	std::shared_ptr<Character> getOwner() const {
		return owner;
	}

	void useCondition(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) {
		if (user != owner) {
			throw CharacterDoesNotOwnItem();
		}

		useLogic(user, target);

		if (isUsableOnce) {
			afterUse();
		}
	}

	virtual void setup() const = 0;
	virtual void useLogic(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) const = 0;

	void afterUse() {
		owner->loseItem(this->shared_from_this());
	}

	void giveDamageTo(const std::shared_ptr<Character> to, int damage) const {
		to->takeDamage(damage);
	}

	void giveHealTo(const std::shared_ptr<Character> to, int heal) const {
		to->heal(heal);
	}
public:
	PhysicalItem(bool isUsableOnce, const std::shared_ptr<Character> owner, const std::string name) : isUsableOnce(isUsableOnce), name(name), owner(owner) {}

	virtual ~PhysicalItem() = default;

	void use(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) {
		useCondition(user, target);
	}

	std::string getName() const {
		return name;
	}

	bool operator > (const PhysicalItem& other) const {
		return (name.compare(other.getName()) > 0);
	}

	bool operator < (const PhysicalItem& other) const {
		return (name.compare(other.getName()) < 0);
	}

	virtual std::ostream& print(std::ostream& out) const = 0;
};

class Weapon : public PhysicalItem {
private:
	int damage;

	void useLogic(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) const override {
		giveDamageTo(target, getDamage());
	}

	void setup() const override {
		auto game = Game::currentGame();
		sysout << owner->getName() << " just obtained a new weapon called " << getName() << ".\n";
	}
public:

	Weapon(const std::shared_ptr<Character> owner, const std::string name, int damage) : PhysicalItem(false, owner, name) {
		if (damage <= 0) {
			throw IllegalDamageValue();
		}

		setup();
	}

	~Weapon() = default;

	int getDamage() const {
		return damage;
	}

	std::ostream& print(std::ostream& out) const override {
		out << getName() << ":" << getDamage() << " ";
		return out;
	}

};

class Potion : public PhysicalItem {
private:
	int healValue;

	void useLogic(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) const override {
		giveHealTo(target, getHealValue());
	}

	void setup() const override {
		auto game = Game::currentGame();
		sysout << owner->getName() << " just obtained a new potion called " << getName() << ".\n";
	}
public:
	
	Potion(const std::shared_ptr<Character> owner, const std::string name, int healValue) : PhysicalItem(true, owner, name) {
		if (healValue <= 0) {
			throw IllegalHealthValue();
		}

		setup();
	}

	~Potion() = default;

	int getHealValue() const {
		return healValue;
	}

	std::ostream& print(std::ostream& out) const override {
		out << getName() << ":" << getHealValue() << " ";
		return out;
	}
};

class Spell : public PhysicalItem {
private:
	std::vector<std::shared_ptr<Character>> allowedTargets;

	void useLogic(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) const override {
		for (auto& allowedTarget : allowedTargets) {
			if (allowedTarget == target) {
				giveDamageTo(target, target->getHp());
				return;
			}
		}

		throw NotAllowedTarget();
	}

	void setup() const override {
		auto game = Game::currentGame();
		sysout << owner->getName() << " just obtained a new spell called " << getName() << ".\n";
	}
public:
	
	Spell(const std::shared_ptr<Character> owner, const std::string name, const std::vector<std::shared_ptr<Character>>& allowedTargets) : PhysicalItem(true, owner, name), allowedTargets(allowedTargets) {
		setup();
	}

	~Spell() = default;

	int getNumAllowedTargets() const {
		return allowedTargets.size();
	}

	std::ostream& print(std::ostream& out) const override {
		out << getName() << ":" << getNumAllowedTargets() << " ";
		return out;
	}
};

using Arsenal = ContainerWithMaxCapacity<Weapon>;
using MedicalBag = ContainerWithMaxCapacity<Potion>;
using SpellBook = ContainerWithMaxCapacity<Spell>;

class WeaponUser : virtual public Character {
protected:
	Arsenal arsenal;
public:

	WeaponUser(const std::string nameString, int healthValue, int capacity) : Character(nameString, healthValue), arsenal(capacity) {}

	~WeaponUser() = default;

	void attack(std::shared_ptr<Character> target, const std::string& weaponName) {
		try {
			auto item = arsenal.get(weaponName);
			item->use(this->shared_from_this(), target);
		}
		catch (const ElementNotFound&) {
			throw CharacterDoesNotOwnItem();
		}
	}

	void showWeapons() const {
		arsenal.show();
	}
};

class PotionUser : virtual public Character {
protected:
	MedicalBag medicalBag;
public:

	PotionUser(const std::string nameString, int healthValue, int capacity) : Character(nameString, healthValue), medicalBag(capacity) {}

	~PotionUser() = default;

	void drink(std::shared_ptr<Character> target, const std::string& potionName) {
		try {
			auto item = medicalBag.get(potionName);
			item->use(this->shared_from_this(), target);
		}
		catch (const ElementNotFound&) {
			throw CharacterDoesNotOwnItem();
		}
	}

	void showPotions() const {
		medicalBag.show();
	}
};

class SpellUser : virtual public Character {
protected:
	SpellBook spellBook;
public:

	SpellUser(const std::string nameString, int healthValue, int capacity) : Character(nameString, healthValue), spellBook(capacity) {}

	~SpellUser() = default;

	void cast(std::shared_ptr<Character> target, const std::string& spellName) {
		try {
			auto item = spellBook.get(spellName);
			item->use(this->shared_from_this(), target);
		}
		catch (const ElementNotFound&) {
			throw CharacterDoesNotOwnItem();
		}
	}

	void showSpells() const {
		spellBook.show();
	}
};

class Fighter : public WeaponUser, public PotionUser {
protected:
	void obtainItem(std::shared_ptr<PhysicalItem> item) override {
		if (dynamic_cast<Weapon*>(item.get())) {
			auto weapon = std::dynamic_pointer_cast<Weapon>(item);
			arsenal.addItem(weapon);
		}
		else if (dynamic_cast<Potion*>(item.get())) {
			auto potion = std::dynamic_pointer_cast<Potion>(item);
			medicalBag.addItem(potion);
		}
		else {
			throw IllegalItemType();
		}
	}

	void loseItem(std::shared_ptr<PhysicalItem> item) override {
		if (arsenal.find(item->getName())) {
			arsenal.removeItem(item->getName());
		}
		else {
			medicalBag.removeItem(item->getName());
		}
		
		item.reset();
	}

	void setup() const override {
		auto game = Game::currentGame();
		sysout << "A new fighter came to town, " << getName() << ".\n";
	}
public:

	Fighter(const std::string nameString, int healthValue) : Character(nameString, healthValue), WeaponUser(nameString, healthValue, maxAllowedWeapons), PotionUser(nameString, healthValue, maxAllowedPotions) {
		setup();
	}

	~Fighter() = default;

	void print(std::ostream& out) const override {
		out << getName() << ":fighter:" << getHp() << " ";
	}

	static int maxAllowedWeapons;
	static int maxAllowedPotions;
};

class Archer : public WeaponUser, public PotionUser, public SpellUser {
protected:
	void obtainItem(std::shared_ptr<PhysicalItem> item) override {
		if (dynamic_cast<Weapon*>(item.get())) {
			auto weapon = std::dynamic_pointer_cast<Weapon>(item);
			arsenal.addItem(weapon);
		}
		else if (dynamic_cast<Potion*>(item.get())) {
			auto potion = std::dynamic_pointer_cast<Potion>(item);
			medicalBag.addItem(potion);
		}
		else if (dynamic_cast<Spell*>(item.get())) {
			auto spell = std::dynamic_pointer_cast<Spell>(item);
			spellBook.addItem(spell);
		}
		else {
			throw IllegalItemType();
		}
	}

	void loseItem(std::shared_ptr<PhysicalItem> item) override {
		if (arsenal.find(item->getName())) {
			arsenal.removeItem(item->getName());
		}
		else if (medicalBag.find(item->getName())) {
			medicalBag.removeItem(item->getName());
		}
		else {
			spellBook.removeItem(item->getName());
		}

		item.reset();
	}

	void setup() const override {
		auto game = Game::currentGame();
		sysout << "A new archer came to town, " << getName() << ".\n";
	}
public:
	Archer(const std::string nameString, int healthValue) : Character(nameString, healthValue), WeaponUser(nameString, healthValue, maxAllowedWeapons), PotionUser(nameString, healthValue, maxAllowedPotions), SpellUser(nameString, healthValue, maxAllowedSpells) {
		setup();
	}

	~Archer() = default;

	void print(std::ostream& out) const override {
		out << getName() << ":archer:" << getHp() << " ";
	}

	static int maxAllowedWeapons;
	static int maxAllowedPotions;
	static int maxAllowedSpells;
};

class Wizard : public PotionUser, public SpellUser {
protected:
	void obtainItem(std::shared_ptr<PhysicalItem> item) override {
		if (dynamic_cast<Potion*>(item.get())) {
			auto potion = std::dynamic_pointer_cast<Potion>(item);
			medicalBag.addItem(potion);
		}
		else if (dynamic_cast<Spell*>(item.get())) {
			auto spell = std::dynamic_pointer_cast<Spell>(item);
			spellBook.addItem(spell);
		}
		else {
			throw IllegalItemType();
		}
	}

	void loseItem(std::shared_ptr<PhysicalItem> item) override {
		if (medicalBag.find(item->getName())) {
			medicalBag.removeItem(item->getName());
		}
		else {
			spellBook.removeItem(item->getName());
		}

		item.reset();
	}

	void setup() const override {
		auto game = Game::currentGame();
		sysout << "A new wizard came to town, " << getName() << ".\n";
	}
public:
	Wizard(const std::string nameString, int healthValue) : Character(nameString, healthValue), PotionUser(nameString, healthValue, maxAllowedPotions), SpellUser(nameString, healthValue, maxAllowedSpells) {
		setup();
	}

	~Wizard() = default;

	void print(std::ostream& out) const override {
		out << getName() << ":wizard:" << getHp() << " ";
	}

	static int maxAllowedPotions;
	static int maxAllowedSpells;
};

// Game Methods

inline std::shared_ptr<Game> Game::game{ nullptr };

std::shared_ptr<Character> Game::getCharacterByName(std::string name) const {
	auto vec = characters.getElements();
	for (auto& character : vec) {
		if (character->getName() == name) {
			return character;
		}
	}

	throw CharacterDoesNotExist();
}

void Game::showCharacters() {
	auto vec = characters.getElements();
	std::sort(vec.begin(), vec.end(), [](const std::shared_ptr<Character> first, const std::shared_ptr<Character> second) {
		return (*first < *second);
	});

	for (auto& character : vec) {
		character->print(output);
	}

	output << std::endl;
}

Game::Game() {
	input.open("input.txt", std::ios_base::in);
	output.open("output.txt", std::ios_base::out);
}

Game::~Game() {
	input.close();
	output.close();
}

void Game::startNewGame() {
	int N; input >> N;
	for (int i = 0; i < N; ++i) {
		std::string first;
		input >> first;
		if (first == "Create") {
			std::string second;
			input >> second;
			if (second == "character") {
				std::string type, name;
				int initHP;
				std::shared_ptr<Character> newChararcter;

				input >> type >> name >> initHP;

				if (type == "fighter") {
					newChararcter = std::make_shared<Fighter>(name, initHP);
				}
				else if (type == "archer") {
					newChararcter = std::make_shared<Archer>(name, initHP);
				}
				else if (type == "wizard") {
					newChararcter = std::make_shared<Fighter>(name, initHP);
				}
				else {
					throw std::runtime_error("Unexpected command");
				}
					
				characters.addItem(newChararcter);
			}
			else if (second == "item") {
				std::string third;
				input >> third;
				if (third == "weapon") {
					try {
						std::string ownerName; input >> ownerName;
						std::string weaponName; input >> weaponName;
						int damageValue; input >> damageValue;
						auto owner = getCharacterByName(ownerName);

						std::shared_ptr<Weapon> newWeapon(new Weapon(owner, weaponName, damageValue));
						owner->obtainItem(newWeapon);
					}
					catch (const CharacterDoesNotExist&) {
						output << "Error caught\n";
						continue;
					}
					catch (const IllegalDamageValue&) {
						output << "Error caught\n";
						continue;
					}
					catch (const FullContainer&) {
						output << "Error caught\n";
						continue;
					}
					catch (const IllegalItemType&) {
						output << "Error caught\n";
						continue;
					}
				}
				else if (third == "potion") {
					try {
						std::string ownerName; input >> ownerName;
						std::string potionName; input >> potionName;
						int healValue; input >> healValue;
						auto owner = getCharacterByName(ownerName);

						std::shared_ptr<Potion> newPotion(new Potion(owner, potionName, healValue));
						owner->obtainItem(newPotion);
					}
					catch (const CharacterDoesNotExist&) {
						output << "Error caught\n";
						continue;
					}
					catch (const IllegalHealthValue&) {
						output << "Error caught\n";
						continue;
					}
					catch (const FullContainer&) {
						output << "Error caught\n";
						continue;
					}
					catch (const IllegalItemType&) {
						output << "Error caught\n";
						continue;
					}
				}
				else if (third == "spell") {
					try {
						std::string ownerName; input >> ownerName;
						std::string spellName; input >> spellName;
						int m; input >> m;

						auto owner = getCharacterByName(ownerName);
							
						std::vector<std::shared_ptr<Character>> allowedTargets;
						for (int j = 0; j < m; ++j) {
							std::string targetName; input >> targetName;
							auto target = getCharacterByName(targetName);
							allowedTargets.push_back(target);
						}
							

						std::shared_ptr<Spell> newSpell(new Spell(owner, spellName, allowedTargets));
						owner->obtainItem(newSpell);
					}
					catch (const CharacterDoesNotExist&) {
						output << "Error caught\n";
						continue;
					}
					catch (const IllegalHealthValue&) {
						output << "Error caught\n";
						continue;
					}
					catch (const FullContainer&) {
						output << "Error caught\n";
						continue;
					}
					catch (const IllegalItemType&) {
						output << "Error caught\n";
						continue;
					}
				}
				else {
					throw std::runtime_error("Unexpected command");
				}
			}
			else {
				throw std::runtime_error("Unexpected command");
			}
		}
		else if (first == "Attack") {
			try {
				std::string attackerName; input >> attackerName;
				std::string targetName; input >> targetName;
				std::string weaponName; input >> weaponName;

				auto attacker = getCharacterByName(attackerName);
				auto target = getCharacterByName(targetName);
					
				if (dynamic_cast<WeaponUser*>(attacker.get())) {
					auto weaponUser = std::dynamic_pointer_cast<WeaponUser>(attacker);
					weaponUser->attack(target, weaponName);
				}
				else {
					throw IllegalItemType();
				}

			}
			catch (const CharacterDoesNotExist&) {
				output << "Error caught\n";
				continue;
			}
			catch (const IllegalItemType&) {
				output << "Error caught\n";
				continue;
			}
			catch (const CharacterDoesNotOwnItem&) {
				output << "Error caught\n";
				continue;
			}
		}
		else if (first == "Cast") {
			try {
				std::string casterName; input >> casterName;
				std::string targetName; input >> targetName;
				std::string spellName; input >> spellName;

				auto caster = getCharacterByName(casterName);
				auto target = getCharacterByName(targetName);

				if (dynamic_cast<SpellUser*>(caster.get())) {
					auto spellUser = std::dynamic_pointer_cast<SpellUser>(caster);
					spellUser->cast(target, spellName);
				}
				else {
					throw IllegalItemType();
				}

			}
			catch (const CharacterDoesNotExist&) {
				output << "Error caught\n";
				continue;
			}
			catch (const IllegalItemType&) {
				output << "Error caught\n";
				continue;
			}
			catch (const CharacterDoesNotOwnItem&) {
				output << "Error caught\n";
				continue;
			}
			catch (const NotAllowedTarget&) {
				output << "Error caught\n";
				continue;
			}
		}
		else if (first == "Drink") {
			try {
				std::string supplierName; input >> supplierName;
				std::string drinkerName; input >> drinkerName;
				std::string potionName; input >> potionName;

				auto supplier = getCharacterByName(supplierName);
				auto drinker = getCharacterByName(drinkerName);

				auto potionUser = std::dynamic_pointer_cast<PotionUser>(supplier);
				potionUser->drink(drinker, potionName);
			}
			catch (const CharacterDoesNotExist&) {
				output << "Error caught\n";
				continue;
			}
			catch (const CharacterDoesNotOwnItem&) {
				output << "Error caught\n";
				continue;
			}
		}
		else if (first == "Dialogue") {
			try {
				std::string speaker; input >> speaker;
				int m; input >> m;
				if (speaker == "Narrator") {
					output << speaker << ": ";
				}
				else {
					getCharacterByName(speaker);
					output << speaker << ": ";
				}
				for (int j = 0; j < m; ++j) {
					std::string word; input >> word;
					output << word << " ";
				}
				output << std::endl;
			}
			catch (const CharacterDoesNotExist&) {
				output << "Error caught\n";
				continue;
			}
		}
		else if (first == "Show") {
			std::string second;
			input >> second;
			if (second == "characters") {
				showCharacters();
			}
			else if (second == "weapons") {
				try {
					std::string characterName; input >> characterName;
					auto owner = getCharacterByName(characterName);
					if (dynamic_cast<WeaponUser*>(owner.get())) {
						auto weaponUser = std::dynamic_pointer_cast<WeaponUser>(owner);
						weaponUser->showWeapons();
					}
					else {
						throw IllegalItemType();
					}
				}
				catch (const CharacterDoesNotExist&) {
					output << "Error caught\n";
					continue;
				}
				catch (const IllegalItemType&) {
					output << "Error caught\n";
					continue;
				}
			}
			else if (second == "potions") {
				try {
					std::string characterName; input >> characterName;
					auto owner = getCharacterByName(characterName);

					auto potionUser = std::dynamic_pointer_cast<PotionUser>(owner);
					potionUser->showPotions();
				}
				catch (const CharacterDoesNotExist&) {
					output << "Error caught\n";
					continue;
				}
			}
			else if (second == "spells") {
				try {
					std::string characterName; input >> characterName;
					auto owner = getCharacterByName(characterName);
					if (dynamic_cast<SpellUser*>(owner.get())) {
						auto spellUser = std::dynamic_pointer_cast<SpellUser>(owner);
						spellUser->showSpells();
					}
					else {
						throw IllegalItemType();
					}
				}
				catch (const CharacterDoesNotExist&) {
					output << "Error caught\n";
					continue;
				}
				catch (const IllegalItemType&) {
					output << "Error caught\n";
					continue;
				}
			}
			else {
				throw std::runtime_error("Unexpected command");
			}
		}
		else {
			throw std::runtime_error("Unexpected command");
		}
	}
}

std::shared_ptr<Game> Game::currentGame() {
	if (game == nullptr) {
		game.reset(new Game());
	}
	return game;
}

void Game::destroyCharacter(std::shared_ptr<Character> ptr) {
	characters.removeItem(ptr);
	ptr.reset();
}

std::ofstream& Game::getOutput() {
	return output;
}

inline int Fighter::maxAllowedWeapons{ 3 };
inline int Fighter::maxAllowedPotions{ 5 };

inline int Archer::maxAllowedWeapons{ 2 };
inline int Archer::maxAllowedPotions{ 3 };
inline int Archer::maxAllowedSpells{ 2 };

inline int Wizard::maxAllowedPotions{ 10 };
inline int Wizard::maxAllowedSpells{ 10 };

int main() {
	auto game = Game::currentGame();
	game->startNewGame();
	return 0;
}
