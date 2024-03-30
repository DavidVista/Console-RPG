#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <type_traits>
#include <memory>

// Output stream
#define out std::cout

// Exceptions
/*
class CharacterDoesNotOwnItem : public std::exception {
public:
	const char* what() {
		return "...";
	}
};
*/

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

// General Classes

class Character : public std::enable_shared_from_this<Character> { // KILLING MECHANISM IS NEEDED
private:
	int healthPoints;
	std::string name;

	void takeDamage(int damage) { // TODO
		healthPoints -= damage;
	}

	void heal(int healValue) { // TODO
		healthPoints += healValue;
	}
protected:
	virtual void loseItem(const std::shared_ptr<PhysicalItem> item) = 0;
	virtual void setup() const = 0;
public:
	friend class PhysicalItem;

	Character(const std::string nameString, int healthValue) : name(nameString), healthPoints(healthValue) {
		setup();
	}

	virtual ~Character() = default;

	std::string getName() const {
		return name;
	}

	int getHp() const {
		return healthPoints;
	}

	bool operator > (const Character& other) const {
		return (name.compare(other.getName()) > 0);
	}

	bool operator > (const Character& other) const {
		return (name.compare(other.getName()) < 0);
	}
};

class PhysicalItem : public std::enable_shared_from_this<PhysicalItem> {
private:
	bool isUsableOnce;
	std::string name;
public:
	PhysicalItem(bool isUsableOnce, const std::shared_ptr<Character> owner, const std::string name) : isUsableOnce(isUsableOnce), owner(owner), name(name) {}

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
};

class Weapon : public PhysicalItem {
private:
	int damage;

	void useLogic(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) const override {
		giveDamageTo(target, getDamage());
	}

	void setup() const override {
		out << owner->getName() << " just obtained a new weapon called " << getName() << ".\n";
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

	static friend std::ostream& operator<<(std::ostream& cout, const Weapon& weapon) {
		cout << weapon.getName() << ":" << weapon.getDamage();
	}

};

class Potion : public PhysicalItem {
private:
	int healValue;

	void useLogic(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) const override {
		giveHealTo(target, getHealValue());
	}

	void setup() const override {
		out << owner->getName() << " just obtained a new potion called " << getName() << ".\n";
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

	static friend std::ostream& operator<<(std::ostream& cout, const Potion& potion) {
		cout << potion.getName() << ":" << potion.getHealValue();
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
		out << owner->getName() << " just obtained a new spell called " << getName() << ".\n";
	}
public:
	
	Spell(const std::shared_ptr<Character> owner, const std::string name, const std::vector<std::shared_ptr<Character>>& allowedTargets) : PhysicalItem(true, owner, name), allowedTargets(allowedTargets) {
		setup();
	}

	~Spell() = default;

	int getNumAllowedTargets() const {
		return allowedTargets.size();
	}

	static friend std::ostream& operator<<(std::ostream& cout, const Spell& spell) {
		cout << spell.getName() << ":" << spell.getNumAllowedTargets();
	}
};

// Concepts

template<typename T>
concept DerivedFromPhysicalItem = std::derived_from<T, PhysicalItem>;

template<typename T>
concept Printable =
	requires (std::ostream & os, T x) { os << x; };

template<typename T>
concept Comparable =
	requires (T x, T y) { x > y; x < y; };

template<typename T>
concept ComparableAndPritable = Printable<T> && Comparable<T>;

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

	unsigned int size() const {
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
		for (unsigned int i = 0; i < elements.size(); ++i) {
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

	Container() : map() {}

	virtual ~Container() {
		for (auto it = map.begin(); it != map.end(); ++it) {
			auto& itemPtr = it->second;
			itemPtr.reset();
		}
	}

	unsigned int size() const {
		return map.size();
	}

	virtual void addItem(std::shared_ptr<T> newItem) {
		map.insert(std::make_pair(newItem->getName(), newItem));
	}

	/*void removeItem(const std::shared_ptr<const T> newItem) {
		removeItem(newItem->getName());
	}*/

	void removeItem(const std::string& itemName) {
		if (map.erase(itemName) == 0) {
			throw ElementNotFound();
		}
	}

	bool find(const std::string& itemName) const {
		return map.contains(itemName);
	}

	std::shared_ptr<T> get(const std::string& itemName) const {
		auto result = map.find(itemName);
		if (result == map.end()) {
			throw ElementNotFound();
		}
		return result->second;
	}

	std::vector<std::shared_ptr<T>> getElements() const {
		std::vector<std::shared_ptr<T>> result(map.size());
		int i = 0;
		for (auto it = map.begin(); it != map.end(); ++it) {
			result[i] = it->second;
		}
		return result;
	}

};

template<ComparableAndPritable T>
class ContainerWithMaxCapacity : public Container<T> {
private:
	unsigned int maxCapacity;
public:
	ContainerWithMaxCapacity(unsigned int maxCapacity) : Container<T>(), maxCapacity(maxCapacity) {}

	~ContainerWithMaxCapacity() = default;

	void addItem(std::shared_ptr<T> newItem) override {
		if (this->size() == maxCapacity) {
			throw FullContainer();
		}
		this->Container<T>::addItem(newItem);
	}

	void show() const {
		auto v = this->getElements();
		std::sort(v.begin(), v.end(), [](const std::shared_ptr<T> first, const std::shared_ptr<T> second){
			return (*first < *second);
		});
		for (auto& element : v) {
			out << *element << " ";
		}
		out << std::endl;
	}
};


using Arsenal = ContainerWithMaxCapacity<Weapon>;
using MedicalBag = ContainerWithMaxCapacity<Potion>;
using SpellBook = ContainerWithMaxCapacity<Spell>;

class WeaponUser : virtual public Character {
protected:
	Arsenal arsenal;
public:

	WeaponUser(const std::string nameString, int healthValue, unsigned int capacity) : Character(nameString, healthValue), arsenal(capacity) {}

	~WeaponUser() = default;

	void attack(std::shared_ptr<Character> target, const std::string& weaponName) {
		try {
			auto item = arsenal.get(weaponName);
			item->use(this->shared_from_this(), target);
		}
		catch (const ElementNotFound e) {
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

	PotionUser(const std::string nameString, int healthValue, unsigned int capacity) : Character(nameString, healthValue), medicalBag(capacity) {}

	~PotionUser() = default;

	void drink(std::shared_ptr<Character> target, const std::string& potionName) {
		try {
			auto item = medicalBag.get(potionName);
			item->use(this->shared_from_this(), target);
		}
		catch (const ElementNotFound e) {
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

	SpellUser(const std::string nameString, int healthValue, unsigned int capacity) : Character(nameString, healthValue), spellBook(capacity) {}

	~SpellUser() = default;

	void cast(std::shared_ptr<Character> target, const std::string& spellName) {
		try {
			auto item = spellBook.get(spellName);
			item->use(this->shared_from_this(), target);
		}
		catch (const ElementNotFound e) {
			throw CharacterDoesNotOwnItem();
		}
	}

	void showSpells() const {
		spellBook.show();
	}
};

class Fighter : public WeaponUser, public PotionUser {
protected:
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
		out << "A new fighter came to town, " << getName() << ".\n";
	}
public:

	Fighter(const std::string nameString, int healthValue) : Character(nameString, healthValue), WeaponUser(nameString, healthValue, maxAllowedWeapons), PotionUser(nameString, healthValue, maxAllowedPotions) {}

	~Fighter() = default;

	static friend std::ostream& operator<<(std::ostream& cout, const Fighter& fighter) {
		cout << fighter.getName() << ":fighter:" << fighter.getHp();
	}

	static unsigned int maxAllowedWeapons;
	static unsigned int maxAllowedPotions;
};

class Archer : public WeaponUser, public PotionUser, public SpellUser {
protected:
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
		out << "A new archer came to town, " << getName() << ".\n";
	}
public:
	Archer(const std::string nameString, int healthValue) : Character(nameString, healthValue), WeaponUser(nameString, healthValue, maxAllowedWeapons), PotionUser(nameString, healthValue, maxAllowedPotions), SpellUser(nameString, healthValue, maxAllowedSpells) {}

	~Archer() = default;

	static friend std::ostream& operator<<(std::ostream& cout, const Archer& archer) {
		cout << archer.getName() << ":archer:" << archer.getHp();
	}

	static unsigned int maxAllowedWeapons;
	static unsigned int maxAllowedPotions;
	static unsigned int maxAllowedSpells;
};

class Wizard : public PotionUser, public SpellUser {
protected:
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
		out << "A new wizard came to town, " << getName() << ".\n";
	}
public:
	Wizard(const std::string nameString, int healthValue) : Character(nameString, healthValue), PotionUser(nameString, healthValue, maxAllowedPotions), SpellUser(nameString, healthValue, maxAllowedSpells) {}

	~Wizard() = default;

	static friend std::ostream& operator<<(std::ostream& cout, const Wizard& wizard) {
		cout << wizard.getName() << ":wizard:" << wizard.getHp();
	}

	static unsigned int maxAllowedPotions;
	static unsigned int maxAllowedSpells;
};


int main() {
	
	return 0;
}
