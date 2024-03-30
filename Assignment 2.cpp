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

// Predeclarations of General Classes

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
	virtual void obtainItemSideEffect(const PhysicalItem& item) = 0;
	virtual void loseItemSideEffect(const PhysicalItem& item) = 0;
public:
	friend class PhysicalItem;

	std::string getName() const {
		return name;
	}

	int getHp() const {
		return healthPoints;
	}

	std::shared_ptr<Character> getptr() {
		return shared_from_this();
	}

	bool operator > (const Character& other) const {
		return (name.compare(other.getName()) > 0);
	}

	bool operator > (const Character& other) const {
		return (name.compare(other.getName()) < 0);
	}
};

class PhysicalItem {
private:
	bool isUsableOnce;
	std::shared_ptr<Character> owner;
	std::string name;
public:
	void use(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) {
		useCondition(user, target);
	}

	std::string getName() const {
		return name;
	}

	virtual void setup() { // Outputs message of creation
		// TODO
	}

	bool operator > (const PhysicalItem& other) const {
		return (name.compare(other.getName()) > 0);
	}

	bool operator < (const PhysicalItem& other) const {
		return (name.compare(other.getName()) < 0);
	}

protected:
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

	virtual void useLogic(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) const = 0;

	void afterUse() {
		// TODO
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
public:
	void setup() override {
		//TODO
	}

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
public:
	void setup() override {
		//TODO
	}

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
public:
	void setup() override {
		//TODO
	}

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
class Container {
private:
	std::vector<std::shared_ptr<T>> elements;
public:

	size_t size() const {
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
		for (size_t i = 0; i < elements.size(); ++i) {
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

template<typename T> requires DerivedFromPhysicalItem<T>
class Container<T> {
private:
	std::unordered_map<std::string, std::shared_ptr<T>> map;
public:

	size_t size() const {
		return map.size();
	}

	virtual void addItem(std::shared_ptr<T> newItem) {
		map.insert(std::make_pair(newItem->getName(), newItem));
	}

	void removeItem(const std::shared_ptr<const T> newItem) {
		removeItem(newItem->getName());
	}

	void removeItem(const std::string& itemName) {
		if (map.erase(itemName) == 0) {
			throw ElementNotFound();
		}
	}

	bool find(const std::shared_ptr<const T> item) const {
		return map.contains(item->getName());
	}

	std::shared_ptr<T> find(const std::string& itemName) const {
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

template<typename T> requires Printable<T> && Comparable<T>
class ContainerWithMaxCapacity : public Container<T> {
private:
	size_t maxCapacity;
public:

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
	void attack(std::shared_ptr<Character> target, const std::string& weaponName) {
		try {
			auto item = arsenal.find(weaponName);
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
	void drink(std::shared_ptr<Character> target, const std::string& potionName) {
		try {
			auto item = medicalBag.find(potionName);
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
	void cast(std::shared_ptr<Character> target, const std::string& spellName) {
		try {
			auto item = spellBook.find(spellName);
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
private:

public:

	static friend std::ostream& operator<<(std::ostream& cout, const Fighter& fighter) {
		cout << fighter.getName() << ":fighter:" << fighter.getHp();
	}

	static size_t maxAllowedWeapons;
	static size_t maxAllowedPotions;
};

// Functions



int main() {
	
	return 0;
}
