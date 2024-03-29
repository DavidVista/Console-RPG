#include <iostream>
#include <memory>

// Predeclarations
class Character;
class PhysicalItem;

// Classes

class Character : public std::enable_shared_from_this<Character> {
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
	virtual void obtainItemSideEffect(PhysicalItem item) = 0;
	virtual void loseItemSideEffect(PhysicalItem item) = 0;
	virtual void print(std::ostream os) const = 0;

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
	
	void use(std::shared_ptr<Character> user, std::shared_ptr<Character> target) {
		useCondition(user, target);
	}

	std::string getName() {
		return name;
	}

	virtual void setup() {
		// TODO
	}

protected:

	std::shared_ptr<Character> getOwner() {
		return owner;
	}

	void useCondition(std::shared_ptr<Character> user, std::shared_ptr<Character> target) {
		if (user != owner) {
			throw std::exception("Character doesn't own an item");
		}

		useLogic(user, target);

		if (isUsableOnce) {
			afterUse();
		}
	}

	virtual void useLogic(std::shared_ptr<Character> user, std::shared_ptr<Character> target) = 0;

	void afterUse() {
		// TODO
	}

	void giveDamageTo(std::shared_ptr<Character> to, int damage) {
		to->takeDamage(damage);
	}

	void giveHealTo(std::shared_ptr<Character> to, int heal) {
		to->heal(heal);
	}

	virtual void print(std::ostream os) const = 0;
};

// Functions



int main() {
	
	return 0;
}
