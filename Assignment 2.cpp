#include <iostream>
#include <vector>
#include <unordered_map>
#include <type_traits>
#include <memory>

// Predeclarations
class Character;
class PhysicalItem;
class Weapon;
class Potion;
class Spell;

template<typename T>
class Container;

// Classes

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
protected:
	std::shared_ptr<Character> getOwner() const {
		return owner;
	}

	void useCondition(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) {
		if (user != owner) {
			throw std::exception("Character doesn't own an item");
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

		throw std::exception("Target is not in the list of allowed targets in casting spells");
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

template<typename T>
class Container {
private:
	std::vector<std::shared_ptr<T>> elements;
public:
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

		throw std::exception("No such element in the container");
	}

	void addItem(std::shared_ptr<T> newItem) {
		elements.push_back(newItem);
	}
};

template<typename T>
concept DerivedFromPhysicalItem = std::derived_from<T, PhysicalItem>;

template<typename T> requires DerivedFromPhysicalItem<T>
class Container<T> {
private:
	std::unordered_map<std::string, std::shared_ptr<T>> map;
public:
	void addItem(std::shared_ptr<T> newItem) {
		map.insert(std::make_pair(newItem->getName(), newItem));
	}

	void removeItem(const std::shared_ptr<const T> newItem) {
		removeItem(newItem->getName());
	}

	void removeItem(const std::string itemName) {
		if (map.erase(itemName) == 0) {
			throw std::exception("No such element in the container");
		}
	}

	bool find(const std::shared_ptr<const T> item) const {
		return map.contains(item->getName());
	}

	std::shared_ptr<T> find(const std::string itemName) const {
		return map.find(itemName)->second;
	}

};

// Functions



int main() {
	
	return 0;
}
