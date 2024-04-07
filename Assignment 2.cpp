#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <fstream>
#include <memory>

// Output stream shortcut

#define sysout game->getOutput()

// Exceptions

class CharacterDoesNotOwnItem: public std::exception
{
public:
    const char *what()
    {
        return "Character doesn't exist";
    }
};

class CharacterDoesNotExist: public std::exception
{
public:
    const char *what()
    {
        return "Character doesn't own an item";
    }
};

class IllegalHealthValue: public std::exception
{
public:
    const char *what()
    {
        return "Negative value or zero for a potion healValue";
    }
};

class IllegalDamageValue: public std::exception
{
public:
    const char *what()
    {
        return "Negative value or zero for a weapon's damageValue";
    }
};

class NotAllowedTarget: public std::exception
{
public:
    const char *what()
    {
        return "Target is not in the list of allowed targets in casting spells";
    }
};

class FullContainer: public std::exception
{
public:
    const char *what()
    {
        return "Arsenal, MedicalBag, or SpellBook are full";
    }
};

class IllegalItemType: public std::exception
{
public:
    const char *what()
    {
        return "Character can't carry or use a certain item";
    }
};

class ElementNotFound: public std::exception
{
public:
    const char *what()
    {
        return "No such element in a container";
    }
};

// Forward Declarations of Classes

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

/// <summary>
/// Concept to accept classes, derived from PhysicalItem.
/// </summary>
template<typename T>
concept DerivedFromPhysicalItem = std::derived_from<T, PhysicalItem>;

/// <summary>
/// Concept to accept classes, which implement print procedure.
/// </summary>
template<typename T>
concept Printable =
    requires(std::ostream & os, T

x) {
x.

print(os);

};

/// <summary>
/// Concept to accept classes, supporting comparing instances.
/// </summary>
template<typename T>
concept Comparable =
    requires(T

x,

T y

) {
x >

y;

x<y;

};

/// <summary>
/// Concept to accept classes, both implementing printing and supporting comparing.
/// </summary>
template<typename T>
concept ComparableAndPrintable = Printable<T> && Comparable<T>;

// Classes

/// <summary>
/// Template class to represent dynamic container of elements.
/// </summary>
/// <typeparam name="T"> template parameter </typeparam>
template<typename T>
class Container
{
private:
    std::vector<std::shared_ptr<T>> elements;
public:

    // Constructor
    Container()
        : elements()
    {}

    // Destructor
    virtual ~Container()
    {

        // Destroying elements in container
        for (auto &itemPtr: elements) {
            itemPtr.reset();
        }
    }

    /// <summary>
    /// Getter for size.
    /// </summary>
    /// <returns>size of the container</returns>
    int size() const
    {
        return elements.size();
    }

    /// <summary>
    /// Function to determine existence of an element in the container.
    /// </summary>
    /// <param name="item"> pointer to the item stored in the container </param>
    /// <returns> true if item is present else false </returns>
    bool find(const std::shared_ptr<const T> item) const
    {
        for (auto &element: elements) {
            if (element == item) {
                return true;
            }
        }

        return false;
    }

    /// <summary>
    /// Removes the item from the container.
    /// </summary>
    /// <param name="newItem"> pointer to the item stored in the container </param>
    void removeItem(const std::shared_ptr<const T> newItem)
    {
        for (int i = 0; i < elements.size(); ++i) {
            if (elements[i] == newItem) {
                elements.erase(elements.begin() + i);
                return;
            }
        }

        throw ElementNotFound();
    }

    /// <summary>
    /// Inserts an element into the container.
    /// </summary>
    /// <param name="newItem"> pointer to the item </param>
    virtual void addItem(std::shared_ptr<T> newItem)
    {
        elements.push_back(newItem);
    }

    /// <summary>
    /// Getter for the vector of elements.
    /// </summary>
    /// <returns> vector of the elements stored in the container </returns>
    std::vector<std::shared_ptr<T>> getElements() const
    {
        return elements;
    }
};

/// <summary>
/// Template specialization of the template class container
/// to represent dynamic container of elements, derived from PhysicalItem.
/// </summary>
/// <typeparam name="T"> template parameter requiring concept DerivedFromPhysicalItem </typeparam>
template<DerivedFromPhysicalItem T>
class Container<T>
{
private:
    // Map to store pair with key of item name and value of pointer to item instance
    std::unordered_map<std::string, std::shared_ptr<T>> map;
public:

    // Constructor
    Container();

    // Destructor
    virtual ~Container();

    /// <summary>
    /// Getter for size.
    /// </summary>
    /// <returns> size of the container </returns>
    int size() const;

    /// <summary>
    /// Inserts an element into the container.
    /// </summary>
    /// <param name="newItem"> pointer to the item instance </param>
    virtual void addItem(std::shared_ptr<T> newItem);

    /// <summary>
    /// Removes the item from the container by the name.
    /// </summary>
    /// <param name="itemName"> name of the item </param>
    void removeItem(const std::string &itemName);

    /// <summary>
    /// Function to determine existence of an element in the container
    /// by the name.
    /// </summary>
    /// <param name="itemName"> name of the item</param>
    /// <returns> true if present in the container else false</returns>
    bool find(const std::string &itemName) const;

    /// <summary>
    /// Gets the item from the container by the name.
    /// </summary>
    /// <param name="itemName"> name of the item </param>
    /// <returns> the pointer to the item instance </returns>
    std::shared_ptr<T> get(const std::string &itemName) const;

    /// <summary>
    /// Getter for the vector of elements.
    /// </summary>
    /// <returns> the vector of pointers to the items stored in the container </returns>
    std::vector<std::shared_ptr<T>> getElements() const;

};

/// <summary>
/// Template class inherits from Container class and represents a dynamic container of elements
/// with limited capacity.
/// </summary>
/// <typeparam name="T"> template parameter requiring concept ComparableAndPrintable</typeparam>
template<ComparableAndPrintable T>
class ContainerWithMaxCapacity: public Container<T>
{
private:
    int maxCapacity;
public:

    // Constructor
    ContainerWithMaxCapacity(int maxCapacity);

    // Destructor
    ~ContainerWithMaxCapacity();

    /// <summary>
    /// Overridden method to insert item with the check for available space.
    /// </summary>
    /// <param name="newItem"> pointer to the item </param>
    void addItem(std::shared_ptr<T> newItem) override;

    /// <summary>
    /// Displays elements in the container.
    /// </summary>
    void show() const;
};

/// <summary>
/// Abstract class Character represents a player
/// in the story.
/// 
/// The class enables creating shared pointers to an instance.
/// </summary>
class Character: public std::enable_shared_from_this<Character>
{
private:
    // Name of a character
    std::string name;

    // Health points of a character
    int healthPoints;

    /// <summary>
    /// Manages taking damage to a character.
    /// </summary>
    /// <param name="damage"> damage value </param>
    void takeDamage(int damage);

    /// <summary>
    /// Manages healing a character.
    /// </summary>
    /// <param name="healValue">heal value</param>
    void heal(int healValue);
protected:

    /// <summary>
    /// Abstract function that manages obtaining an item by a character.
    /// </summary>
    /// <param name="item"> pointer to the item </param>
    virtual void obtainItem(std::shared_ptr<PhysicalItem> item) = 0;

    /// <summary>
    /// Abstract function that manages losing an item by a character.
    /// </summary>
    /// <param name="item"> pointer to the item </param>
    virtual void loseItem(const std::shared_ptr<PhysicalItem> item) = 0;
public:

    // Allows PhysicalItem and Game classes to access private and protected members of class Character
    friend class PhysicalItem;
    friend class Game;

    // Constructor
    Character(const std::string nameString, int healthValue);

    // Destructor
    virtual ~Character();

    /// <summary>
    /// Getter for name.
    /// </summary>
    /// <returns> the name of the character </returns>
    std::string getName() const;

    /// <summary>
    /// Getter for health points.
    /// </summary>
    /// <returns>current health points of the character </returns>
    int getHp() const;

    /// <summary>
    /// Operator to compare two Characters.
    /// </summary>
    /// <param name="other"> comparing character </param>
    /// <returns>true if compared character is bigger than comparing one</returns>
    bool operator>(const Character &other) const;

    /// <summary>
    /// Operator to compare two Characters.
    /// </summary>
    /// <param name="other"> comparing character </param>
    /// <returns> true if the compared character is less than the comparing one </returns>
    bool operator<(const Character &other) const;

    /// <summary>
    /// Abstract function to print information about a character to the output stream.
    /// </summary>
    /// <param name="out"> reference to the output stream </param>
    virtual void print(std::ostream &out) const = 0;
};

/// <summary>
/// Singleton class Game to represent a
/// single game session.
/// </summary>
class Game
{
private:
    // Singleton instance
    static std::shared_ptr<Game> game;

    // Container of alive characters
    Container<Character> characters;

    // Input stream
    std::ifstream input;

    // Output stream
    std::ofstream output;

    /// <summary>
    /// Function to get Character instance from the container.
    /// </summary>
    /// <param name="name"> name of the character </param>
    /// <returns> pointer to the character instance </returns>
    std::shared_ptr<Character> getCharacterByName(std::string name) const;

    /// <summary>
    /// Displays information about alive characters in the lexicographical order of names.
    /// </summary>
    void showCharacters();

    // Private constructor
    Game();
public:

    /// <summary>
    /// Entry point of the game.
    /// Deals with reading input and processing commands.
    /// </summary>
    void startNewGame();

    /// <summary>
    /// Instance getter.
    /// </summary>
    /// <returns> pointer to the Game instance </returns>
    static std::shared_ptr<Game> currentGame();

    /// <summary>
    /// Procedure to remove a character from the game.
    /// </summary>
    /// <param name="ptr"> pointer to the character instance </param>
    void destroyCharacter(std::shared_ptr<Character> ptr);

    /// <summary>
    /// Getter for the output stream.
    /// </summary>
    /// <returns> the reference to the output stream </returns>
    std::ofstream &getOutput();
};

// Container for Physical Items Methods

template<DerivedFromPhysicalItem T>
Container<T>::Container()
    : map()
{}

template<DerivedFromPhysicalItem T>
Container<T>::~Container()
{

    // Destroying elements stored in the container
    for (auto it = map.begin(); it != map.end(); ++it) {
        auto &itemPtr = it->second;
        itemPtr.reset();
    }
}

template<DerivedFromPhysicalItem T>
int Container<T>::size() const
{
    return map.size();
}

template<DerivedFromPhysicalItem T>
void Container<T>::addItem(std::shared_ptr<T> newItem)
{
    map.insert(std::make_pair(newItem->getName(), newItem));
}

template<DerivedFromPhysicalItem T>
void Container<T>::removeItem(const std::string &itemName)
{
    if (map.erase(itemName) == 0) {
        throw ElementNotFound();
    }
}

template<DerivedFromPhysicalItem T>
bool Container<T>::find(const std::string &itemName) const
{
    return map.contains(itemName);
}

template<DerivedFromPhysicalItem T>
std::shared_ptr<T> Container<T>::get(const std::string &itemName) const
{
    auto result = map.find(itemName);
    if (result == map.end()) {
        throw ElementNotFound();
    }
    return result->second;
}

template<DerivedFromPhysicalItem T>
std::vector<std::shared_ptr<T>> Container<T>::getElements() const
{
    std::vector<std::shared_ptr<T>> result;
    for (auto it = map.begin(); it != map.end(); ++it) {
        result.push_back(it->second);
    }
    return result;
}

// Container with Max Capacity Methods

template<ComparableAndPritable T>
ContainerWithMaxCapacity<T>::ContainerWithMaxCapacity(int maxCapacity)
    : Container<T>(), maxCapacity(maxCapacity)
{}

template<ComparableAndPritable T>
ContainerWithMaxCapacity<T>::~ContainerWithMaxCapacity() = default;

template<ComparableAndPritable T>
void ContainerWithMaxCapacity<T>::addItem(std::shared_ptr<T> newItem)
{

    // Additional check for available space
    if (this->size() == maxCapacity) {
        throw FullContainer();
    }
    this->Container<T>::addItem(newItem);
}

template<ComparableAndPritable T>
void ContainerWithMaxCapacity<T>::show() const
{

    // Vector of the elements
    auto v = this->getElements();

    // Instance of the game
    auto game = Game::currentGame();

    // Sorting elements
    std::sort(v.begin(), v.end(), [](const std::shared_ptr<T> first, const std::shared_ptr<T> second)
    {
        return (*first < *second);
    });

    // Printing elements
    for (auto &element: v) {
        element->print(sysout);
    }
    sysout << std::endl;
}

// Character Methods

void Character::takeDamage(int damage)
{
    healthPoints -= damage;

    // Check whether a character is alive
    if (healthPoints <= 0) {
        auto game = Game::currentGame();

        // Remove dead character from the game
        game->destroyCharacter(this->shared_from_this());
    }
}

void Character::heal(int healValue)
{
    healthPoints += healValue;
}

Character::Character(const std::string nameString, int healthValue)
    : name(nameString), healthPoints(healthValue)
{}

Character::~Character() = default;

std::string Character::getName() const
{
    return name;
}

int Character::getHp() const
{
    return healthPoints;
}

bool Character::operator>(const Character &other) const
{
    // Lexicographical comparison
    return (name.compare(other.getName()) > 0);
}

bool Character::operator<(const Character &other) const
{
    // Lexicographical comparison
    return (name.compare(other.getName()) < 0);
}

/// <summary>
/// Abstract class PhysicalItem represents an item that
/// a character can store or use during the game session.
/// 
/// The class enables creating shared pointers from an instance.
/// </summary>
class PhysicalItem: public std::enable_shared_from_this<PhysicalItem>
{
private:
    // States whether an item cannot be used more than once
    bool isUsableOnce;

    // Name of an item
    std::string name;
protected:

    // The owner of an item
    std::shared_ptr<Character> owner;

    /// <summary>
    /// Getter for the owner.
    /// </summary>
    /// <returns> pointer to the character instance </returns>
    std::shared_ptr<Character> getOwner() const
    {
        return owner;
    }

    /// <summary>
    /// Handles checks for the use of an item.
    /// </summary>
    /// <param name="user"> owner of the item </param>
    /// <param name="target"> target to use item on </param>
    void useCondition(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target)
    {

        // Owner check
        if (user != owner) {
            throw CharacterDoesNotOwnItem();
        }

        // Applying item
        useLogic(user, target);

        // Destroying an item after use
        if (isUsableOnce) {
            afterUse();
        }
    }

    /// <summary>
    /// Abstract function to apply element of a user on a target.
    /// </summary>
    /// <param name="user">owner of the item</param>
    /// <param name="target">target to use item on</param>
    virtual void useLogic(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) const = 0;

    /// <summary>
    /// Deals with item destruction after use.
    /// </summary>
    void afterUse()
    {
        owner->loseItem(this->shared_from_this());
    }

    /// <summary>
    /// Deals with giving damage to a given character.
    /// </summary>
    /// <param name="to"> receiver of damage </param>
    /// <param name="damage"> damage value </param>
    void giveDamageTo(const std::shared_ptr<Character> to, int damage) const
    {
        to->takeDamage(damage);
    }

    /// <summary>
    /// Deals with healing a given character.
    /// </summary>
    /// <param name="to"> receiver of heal</param>
    /// <param name="heal"> heal value </param>
    void giveHealTo(const std::shared_ptr<Character> to, int heal) const
    {
        to->heal(heal);
    }
public:

    // Constructor
    PhysicalItem(bool isUsableOnce, const std::shared_ptr<Character> owner, const std::string name)
        : isUsableOnce(isUsableOnce), name(name), owner(owner)
    {}

    // Destructor
    virtual ~PhysicalItem() = default;

    /// <summary>
    /// Function that provides the use of an item.
    /// </summary>
    /// <param name="user"> owner of the item </param>
    /// <param name="target"> target to use item on </param>
    void use(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target)
    {
        useCondition(user, target);
    }

    /// <summary>
    /// Getter for the name.
    /// </summary>
    /// <returns> name of the item </returns>
    std::string getName() const
    {
        return name;
    }

    /// <summary>
    /// Operator to compare two PhysicalItems.
    /// </summary>
    /// <param name="other">comparing item</param>
    /// <returns> true if the compared item is bigger than the comparing item </returns>
    bool operator>(const PhysicalItem &other) const
    {
        return (name.compare(other.getName()) > 0);
    }

    /// <summary>
    /// Operator to compare two PhysicalItems.
    /// </summary>
    /// <param name="other">comparing item</param>
    /// <returns> true if the compared item is less than the comparing item </returns>
    bool operator<(const PhysicalItem &other) const
    {
        return (name.compare(other.getName()) < 0);
    }

    /// <summary>
    /// Abstract function to print information about an item
    /// to the output stream.
    /// </summary>
    /// <param name="out"> reference to the output stream</param>
    virtual void print(std::ostream &out) const = 0;
};

/// <summary>
/// Class Weapon inherits from PhysicalItem and
/// represents an item that can give damage to
/// characters.
/// </summary>
class Weapon: public PhysicalItem
{
private:
    int damage;

    /// <summary>
    /// Implementation of the abstract function that
    /// gives damage to target.
    /// </summary>
    /// <param name="user"> attacker</param>
    /// <param name="target"> receiver of damage </param>
    void useLogic(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) const override
    {
        auto game = Game::currentGame();
        sysout << user->getName() << " attacks " << target->getName() << " with their " << getName() << "!\n";
        giveDamageTo(target, getDamage());
    }
public:

    // Constructor
    Weapon(const std::shared_ptr<Character> owner, const std::string name, int damage)
        : PhysicalItem(false, owner, name), damage(damage)
    {

        // Check for legal damage value
        if (damage <= 0) {
            throw IllegalDamageValue();
        }
    }

    // Destructor
    ~Weapon() = default;

    /// <summary>
    /// Getter for damage value.
    /// </summary>
    /// <returns> damage value </returns>
    int getDamage() const
    {
        return damage;
    }

    /// <summary>
    /// Implementation of the print function.
    /// </summary>
    /// <param name="out"> reference to the output stream </param>
    void print(std::ostream &out) const override
    {
        out << getName() << ":" << getDamage() << " ";
    }

};

/// <summary>
/// Class Potion inherits from PhysicalItem and
/// represents an item that can heal characters.
/// </summary>
class Potion: public PhysicalItem
{
private:
    int healValue;

    /// <summary>
    /// Implementation of the abstract function that
    /// heals target.
    /// </summary>
    /// <param name="user">healer</param>
    /// <param name="target">receiver of heal</param>
    void useLogic(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) const override
    {
        auto game = Game::currentGame();
        sysout << target->getName() << " drinks " << getName() << " from " << user->getName() << ".\n";
        giveHealTo(target, getHealValue());
    }
public:

    // Constructor
    Potion(const std::shared_ptr<Character> owner, const std::string name, int healValue)
        : PhysicalItem(true, owner, name), healValue(healValue)
    {
        if (healValue <= 0) {
            throw IllegalHealthValue();
        }
    }

    // Destructor
    ~Potion() = default;

    /// <summary>
    /// Getter for health value.
    /// </summary>
    /// <returns> heal value </returns>
    int getHealValue() const
    {
        return healValue;
    }

    /// <summary>
    /// Implementation of the print function.
    /// </summary>
    /// <param name="out"> reference to the output stream </param>
    void print(std::ostream &out) const override
    {
        out << getName() << ":" << getHealValue() << " ";
    }
};

/// <summary>
/// Class Spell inherits from PhysicalItem and
/// represents an item that kill characters.
/// </summary>
class Spell: public PhysicalItem
{
private:

    // List of instances that a spell can be cast on
    std::vector<std::shared_ptr<Character>> allowedTargets;

    /// <summary>
    /// Implementation of the abstract function that
    /// casts a spell on target.
    /// </summary>
    /// <param name="user"> caster </param>
    /// <param name="target"> target to cast spell on</param>
    void useLogic(const std::shared_ptr<const Character> user, std::shared_ptr<Character> target) const override
    {
        for (auto &allowedTarget: allowedTargets) {
            if (allowedTarget == target) {
                auto game = Game::currentGame();
                sysout << user->getName() << " casts " << getName() << " on " << target->getName() << "!\n";
                giveDamageTo(target, target->getHp());
                return;
            }
        }

        throw NotAllowedTarget();
    }
public:

    // Constructor
    Spell(const std::shared_ptr<Character> owner,
          const std::string name,
          const std::vector<std::shared_ptr<Character>> &allowedTargets)
        : PhysicalItem(true, owner, name), allowedTargets(allowedTargets)
    {}

    // Destructor
    ~Spell() = default;

    /// <summary>
    /// Getter for the number of allowed targets.
    /// </summary>
    /// <returns> number of allowed targets </returns>
    int getNumAllowedTargets() const
    {
        return allowedTargets.size();
    }

    /// <summary>
    /// Implementation of the print function.
    /// </summary>
    /// <param name="out"> reference to the output stream </param>
    void print(std::ostream &out) const override
    {
        out << getName() << ":" << getNumAllowedTargets() << " ";
    }
};

// Bindings of the Containers

using Arsenal = ContainerWithMaxCapacity<Weapon>;
using MedicalBag = ContainerWithMaxCapacity<Potion>;
using SpellBook = ContainerWithMaxCapacity<Spell>;

/// <summary>
/// Class WeaponUser represents a character that is
/// able to use weapons.
/// </summary>
class WeaponUser: virtual public Character
{
protected:

    // Arsenal of weapons
    Arsenal arsenal;
public:

    // Constructor
    WeaponUser(const std::string nameString, int healthValue, int capacity)
        : Character(nameString, healthValue), arsenal(capacity)
    {}

    // Destructor
    ~WeaponUser() = default;

    /// <summary>
    /// The function implements attack on the given character.
    /// </summary>
    /// <param name="target">receiver of damage</param>
    /// <param name="weaponName">name of the weapon</param>
    void attack(std::shared_ptr<Character> target, const std::string &weaponName)
    {
        try {
            auto item = arsenal.get(weaponName);
            item->use(this->shared_from_this(), target);
        }
        catch (const ElementNotFound &) {
            throw CharacterDoesNotOwnItem();
        }
    }

    /// <summary>
    /// Displays all weapons in the arsenal.
    /// </summary>
    void showWeapons() const
    {
        arsenal.show();
    }
};

/// <summary>
/// Class PotionUser represents a character that is
/// able to use potions.
/// </summary>
class PotionUser: virtual public Character
{
protected:

    // Container of potions
    MedicalBag medicalBag;
public:

    // Constructor
    PotionUser(const std::string nameString, int healthValue, int capacity)
        : Character(nameString, healthValue), medicalBag(capacity)
    {}

    // Destructor
    ~PotionUser() = default;

    /// <summary>
    /// The function implements drinking of a potion by a target.
    /// </summary>
    /// <param name="target">receiver of heal</param>
    /// <param name="potionName">name of the potion</param>
    void drink(std::shared_ptr<Character> target, const std::string &potionName)
    {
        try {
            auto item = medicalBag.get(potionName);
            item->use(this->shared_from_this(), target);
        }
        catch (const ElementNotFound &) {
            throw CharacterDoesNotOwnItem();
        }
    }

    /// <summary>
    /// Displays all potions in the medicalBag.
    /// </summary>
    void showPotions() const
    {
        medicalBag.show();
    }
};

/// <summary>
/// Class SpellUser represents a character that is
/// able to cast spells.
/// </summary>
class SpellUser: virtual public Character
{
protected:

    // Container of spells
    SpellBook spellBook;
public:

    // Constructor
    SpellUser(const std::string nameString, int healthValue, int capacity)
        : Character(nameString, healthValue), spellBook(capacity)
    {}

    // Destructor
    ~SpellUser() = default;

    /// <summary>
    /// The function implements cast on a target.
    /// </summary>
    /// <param name="target"> target to cast spell on</param>
    /// <param name="spellName"> name of the spell</param>
    void cast(std::shared_ptr<Character> target, const std::string &spellName)
    {
        try {
            auto item = spellBook.get(spellName);
            item->use(this->shared_from_this(), target);
        }
        catch (const ElementNotFound &) {
            throw CharacterDoesNotOwnItem();
        }
    }

    /// <summary>
    /// Displays all spells from the spellBook.
    /// </summary>
    void showSpells() const
    {
        spellBook.show();
    }
};

/// <summary>
/// Class Fighter represents a character that can
/// use weapons and potions.
/// </summary>
class Fighter: public WeaponUser, public PotionUser
{
protected:

    /// <summary>
    /// Implementation of the abstract function that inserts the item
    /// into either the arsenal or the medicalBag.
    /// </summary>
    /// <param name="item"> pointer to the item </param>
    void obtainItem(std::shared_ptr<PhysicalItem> item) override
    {

        // Cast of item to Weapon
        if (dynamic_cast<Weapon *>(item.get())) {
            auto weapon = std::dynamic_pointer_cast<Weapon>(item);
            arsenal.addItem(weapon);
        }
            // Cast of item to Potion
        else if (dynamic_cast<Potion *>(item.get())) {
            auto potion = std::dynamic_pointer_cast<Potion>(item);
            medicalBag.addItem(potion);
        }
            // Element cannot be used by a fighter
        else {
            throw IllegalItemType();
        }
    }

    /// <summary>
    /// Implementation of the abstract function that removes the item
    /// from either the arsenal or the medicalBag.
    /// </summary>
    /// <param name="item"> pointer to the item</param>
    void loseItem(std::shared_ptr<PhysicalItem> item) override
    {

        // Find the item in the arsenal
        if (arsenal.find(item->getName())) {
            arsenal.removeItem(item->getName());
        }
            // Find the item in the medicalBag
        else {
            medicalBag.removeItem(item->getName());
        }

        // Release data
        item.reset();
    }
public:

    // Constructor
    Fighter(const std::string nameString, int healthValue)
        : Character(nameString, healthValue), WeaponUser(nameString, healthValue, maxAllowedWeapons),
          PotionUser(nameString, healthValue, maxAllowedPotions)
    {}

    // Destructor
    ~Fighter() = default;

    /// <summary>
    /// Implementation of the print function.
    /// </summary>
    /// <param name="out"> reference to the output stream </param>
    void print(std::ostream &out) const override
    {
        out << getName() << ":fighter:" << getHp() << " ";
    }

    // Maximum allowed number of weapons
    static int maxAllowedWeapons;

    // Maximum allowed number of potions
    static int maxAllowedPotions;
};

/// <summary>
/// Class Archer represents a character that can
/// use weapons, potions, and spells.
/// </summary>
class Archer: public WeaponUser, public PotionUser, public SpellUser
{
protected:

    /// <summary>
    /// Implementation of the abstract function that inserts the item
    /// into either the arsenal, the medicalBag, or the spellBook.
    /// </summary>
    /// <param name="item">pointer to the item </param>
    void obtainItem(std::shared_ptr<PhysicalItem> item) override
    {

        // Cast the item to the Weapon
        if (dynamic_cast<Weapon *>(item.get())) {
            auto weapon = std::dynamic_pointer_cast<Weapon>(item);
            arsenal.addItem(weapon);
        }
            // Cast the item to the Potion
        else if (dynamic_cast<Potion *>(item.get())) {
            auto potion = std::dynamic_pointer_cast<Potion>(item);
            medicalBag.addItem(potion);
        }
            // Cast the item to the Spell
        else if (dynamic_cast<Spell *>(item.get())) {
            auto spell = std::dynamic_pointer_cast<Spell>(item);
            spellBook.addItem(spell);
        }
        else {
            throw IllegalItemType();
        }
    }

    /// <summary>
    /// Implementation of the abstract function that removes the item
    /// from either the arsenal, the medicalBag, or the spellBook.
    /// </summary>
    /// <param name="item"> pointer to the item </param>
    void loseItem(std::shared_ptr<PhysicalItem> item) override
    {

        // Find the item in the arsenal
        if (arsenal.find(item->getName())) {
            arsenal.removeItem(item->getName());
        }
            // Find the item in the medicalBag
        else if (medicalBag.find(item->getName())) {
            medicalBag.removeItem(item->getName());
        }
            // Find the item in the spellBook
        else {
            spellBook.removeItem(item->getName());
        }

        item.reset();
    }
public:

    // Constructor
    Archer(const std::string nameString, int healthValue)
        : Character(nameString, healthValue), WeaponUser(nameString, healthValue, maxAllowedWeapons),
          PotionUser(nameString, healthValue, maxAllowedPotions), SpellUser(nameString, healthValue, maxAllowedSpells)
    {}

    // Destructor
    ~Archer() = default;

    /// <summary>
    /// Implementation of the print function.
    /// </summary>
    /// <param name="out"> reference to the output stream </param>
    void print(std::ostream &out) const override
    {
        out << getName() << ":archer:" << getHp() << " ";
    }

    // Maximum allowed number of weapons
    static int maxAllowedWeapons;

    // Maximum allowed number of potions
    static int maxAllowedPotions;

    // Maximum allowed number of spells
    static int maxAllowedSpells;
};

/// <summary>
/// Class Wizard represents a character that can
/// use potions and spells.
/// </summary>
class Wizard: public PotionUser, public SpellUser
{
protected:

    /// <summary>
    /// Implementation of the abstract function that inserts the item
    /// into either the medicalBag or the spellBook.
    /// </summary>
    /// <param name="item"></param>
    void obtainItem(std::shared_ptr<PhysicalItem> item) override
    {
        // Cast to Potion
        if (dynamic_cast<Potion *>(item.get())) {
            auto potion = std::dynamic_pointer_cast<Potion>(item);
            medicalBag.addItem(potion);
        }
            // Cast to Spell
        else if (dynamic_cast<Spell *>(item.get())) {
            auto spell = std::dynamic_pointer_cast<Spell>(item);
            spellBook.addItem(spell);
        }
            // Wizard cannot use such item
        else {
            throw IllegalItemType();
        }
    }

    /// <summary>
    /// Implementation of the abstract function that removes the item
    /// from either the medicalBag or the spellBook.
    /// </summary>
    /// <param name="item"></param>
    void loseItem(std::shared_ptr<PhysicalItem> item) override
    {
        // Find the item in the medicalBag
        if (medicalBag.find(item->getName())) {
            medicalBag.removeItem(item->getName());
        }
            // Find the item in the spellBook
        else {
            spellBook.removeItem(item->getName());
        }

        // Release data
        item.reset();
    }
public:

    // Constructor
    Wizard(const std::string nameString, int healthValue)
        : Character(nameString, healthValue), PotionUser(nameString, healthValue, maxAllowedPotions),
          SpellUser(nameString, healthValue, maxAllowedSpells)
    {}

    // Destructor
    ~Wizard() = default;

    /// <summary>
    /// Implementation of the print function.
    /// </summary>
    /// <param name="out"> reference to the output stream </param>
    void print(std::ostream &out) const override
    {
        out << getName() << ":wizard:" << getHp() << " ";
    }

    // Maximum allowed number of potions
    static int maxAllowedPotions;

    // Maximum allowed number of spells
    static int maxAllowedSpells;
};

// Game Methods

inline std::shared_ptr<Game> Game::game{nullptr};

std::shared_ptr<Character> Game::getCharacterByName(std::string name) const
{
    auto vec = characters.getElements();
    for (auto &character: vec) {
        if (character->getName() == name) {
            return character;
        }
    }

    throw CharacterDoesNotExist();
}

void Game::showCharacters()
{

    // Get the vector of alive characters
    auto vec = characters.getElements();

    // Sort characters by name
    std::sort(vec.begin(),
              vec.end(),
              [](const std::shared_ptr<Character> first, const std::shared_ptr<Character> second)
              {
                  return (*first < *second);
              });

    // Output characters information
    for (auto &character: vec) {
        character->print(output);
    }

    output << std::endl;
}

Game::Game()
{
    // Input stream
    input.open("input.txt");

    // Output stream
    output.open("output.txt");
}

void Game::startNewGame()
{

    // Processing Input

    int N;
    input >> N;
    for (int i = 0; i < N; ++i) {
        std::string first;
        input >> first;
        if (first == "Create") {
            std::string second;
            input >> second;
            if (second == "character") {
                std::string type, name;
                int initHP;
                std::shared_ptr<Character> newCharacter;

                input >> type >> name >> initHP;

                if (type == "fighter") {
                    newCharacter = std::make_shared<Fighter>(name, initHP);
                    output << "A new fighter came to town, " << name << ".\n";
                }
                else if (type == "archer") {
                    newCharacter = std::make_shared<Archer>(name, initHP);
                    output << "A new archer came to town, " << name << ".\n";
                }
                else if (type == "wizard") {
                    newCharacter = std::make_shared<Wizard>(name, initHP);
                    output << "A new wizard came to town, " << name << ".\n";
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
                        std::string ownerName;
                        input >> ownerName;
                        std::string weaponName;
                        input >> weaponName;
                        int damageValue;
                        input >> damageValue;
                        auto owner = getCharacterByName(ownerName);

                        std::shared_ptr<Weapon> newWeapon = std::make_shared<Weapon>(owner, weaponName, damageValue);
                        owner->obtainItem(newWeapon);
                        output << ownerName << " just obtained a new weapon called " << weaponName << ".\n";
                    }
                    catch (const CharacterDoesNotExist &) {
                        output << "Error caught\n";
                        continue;
                    }
                    catch (const IllegalDamageValue &) {
                        output << "Error caught\n";
                        continue;
                    }
                    catch (const FullContainer &) {
                        output << "Error caught\n";
                        continue;
                    }
                    catch (const IllegalItemType &) {
                        output << "Error caught\n";
                        continue;
                    }
                }
                else if (third == "potion") {
                    try {
                        std::string ownerName;
                        input >> ownerName;
                        std::string potionName;
                        input >> potionName;
                        int healValue;
                        input >> healValue;
                        auto owner = getCharacterByName(ownerName);

                        std::shared_ptr<Potion> newPotion = std::make_shared<Potion>(owner, potionName, healValue);
                        owner->obtainItem(newPotion);
                        output << ownerName << " just obtained a new potion called " << potionName << ".\n";
                    }
                    catch (const CharacterDoesNotExist &) {
                        output << "Error caught\n";
                        continue;
                    }
                    catch (const IllegalHealthValue &) {
                        output << "Error caught\n";
                        continue;
                    }
                    catch (const FullContainer &) {
                        output << "Error caught\n";
                        continue;
                    }
                    catch (const IllegalItemType &) {
                        output << "Error caught\n";
                        continue;
                    }
                }
                else if (third == "spell") {
                    try {
                        std::string ownerName;
                        input >> ownerName;
                        std::string spellName;
                        input >> spellName;
                        int m;
                        input >> m;

                        std::vector<std::string> targetNames;

                        for (int j = 0; j < m; ++j) {
                            std::string targetName;
                            input >> targetName;
                            targetNames.push_back(targetName);
                        }

                        auto owner = getCharacterByName(ownerName);

                        std::vector<std::shared_ptr<Character>> allowedTargets;

                        for (int j = 0; j < m; ++j) {
                            std::string targetName = targetNames[j];
                            auto target = getCharacterByName(targetName);
                            allowedTargets.push_back(target);
                        }

                        std::shared_ptr<Spell> newSpell = std::make_shared<Spell>(owner, spellName, allowedTargets);
                        owner->obtainItem(newSpell);
                        output << ownerName << " just obtained a new spell called " << spellName << ".\n";
                    }
                    catch (const CharacterDoesNotExist &) {
                        output << "Error caught\n";
                        continue;
                    }
                    catch (const FullContainer &) {
                        output << "Error caught\n";
                        continue;
                    }
                    catch (const IllegalItemType &) {
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
                std::string attackerName;
                input >> attackerName;
                std::string targetName;
                input >> targetName;
                std::string weaponName;
                input >> weaponName;

                auto attacker = getCharacterByName(attackerName);
                auto target = getCharacterByName(targetName);

                // Check whether the character can use weapons
                if (dynamic_cast<WeaponUser *>(attacker.get())) {
                    auto weaponUser = std::dynamic_pointer_cast<WeaponUser>(attacker);
                    weaponUser->attack(target, weaponName);
                }
                else {
                    throw IllegalItemType();
                }

            }
            catch (const CharacterDoesNotExist &) {
                output << "Error caught\n";
                continue;
            }
            catch (const IllegalItemType &) {
                output << "Error caught\n";
                continue;
            }
            catch (const CharacterDoesNotOwnItem &) {
                output << "Error caught\n";
                continue;
            }
        }
        else if (first == "Cast") {
            try {
                std::string casterName;
                input >> casterName;
                std::string targetName;
                input >> targetName;
                std::string spellName;
                input >> spellName;

                auto caster = getCharacterByName(casterName);
                auto target = getCharacterByName(targetName);

                // Check whether the character can use spells
                if (dynamic_cast<SpellUser *>(caster.get())) {
                    auto spellUser = std::dynamic_pointer_cast<SpellUser>(caster);
                    spellUser->cast(target, spellName);
                }
                else {
                    throw IllegalItemType();
                }

            }
            catch (const CharacterDoesNotExist &) {
                output << "Error caught\n";
                continue;
            }
            catch (const IllegalItemType &) {
                output << "Error caught\n";
                continue;
            }
            catch (const CharacterDoesNotOwnItem &) {
                output << "Error caught\n";
                continue;
            }
            catch (const NotAllowedTarget &) {
                output << "Error caught\n";
                continue;
            }
        }
        else if (first == "Drink") {
            try {
                std::string supplierName;
                input >> supplierName;
                std::string drinkerName;
                input >> drinkerName;
                std::string potionName;
                input >> potionName;

                auto supplier = getCharacterByName(supplierName);
                auto drinker = getCharacterByName(drinkerName);

                auto potionUser = std::dynamic_pointer_cast<PotionUser>(supplier);
                potionUser->drink(drinker, potionName);
            }
            catch (const CharacterDoesNotExist &) {
                output << "Error caught\n";
                continue;
            }
            catch (const CharacterDoesNotOwnItem &) {
                output << "Error caught\n";
                continue;
            }
        }
        else if (first == "Dialogue") {
            try {
                std::string speaker;
                input >> speaker;
                int m;
                input >> m;
                std::string speech;

                for (int j = 0; j < m; ++j) {
                    std::string word;
                    input >> word;
                    speech += word + " ";
                }

                if (speaker == "Narrator") {
                    output << speaker << ": ";
                }
                else {
                    getCharacterByName(speaker);
                    output << speaker << ": ";
                }

                output << speech << std::endl;
            }
            catch (const CharacterDoesNotExist &) {
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
                    std::string characterName;
                    input >> characterName;
                    auto owner = getCharacterByName(characterName);

                    // Check whether the character can use weapons
                    if (dynamic_cast<WeaponUser *>(owner.get())) {
                        auto weaponUser = std::dynamic_pointer_cast<WeaponUser>(owner);
                        weaponUser->showWeapons();
                    }
                    else {
                        throw IllegalItemType();
                    }
                }
                catch (const CharacterDoesNotExist &) {
                    output << "Error caught\n";
                    continue;
                }
                catch (const IllegalItemType &) {
                    output << "Error caught\n";
                    continue;
                }
            }
            else if (second == "potions") {
                try {
                    std::string characterName;
                    input >> characterName;
                    auto owner = getCharacterByName(characterName);

                    auto potionUser = std::dynamic_pointer_cast<PotionUser>(owner);
                    potionUser->showPotions();
                }
                catch (const CharacterDoesNotExist &) {
                    output << "Error caught\n";
                    continue;
                }
            }
            else if (second == "spells") {
                try {
                    std::string characterName;
                    input >> characterName;
                    auto owner = getCharacterByName(characterName);

                    // Check whether the character can use spells
                    if (dynamic_cast<SpellUser *>(owner.get())) {
                        auto spellUser = std::dynamic_pointer_cast<SpellUser>(owner);
                        spellUser->showSpells();
                    }
                    else {
                        throw IllegalItemType();
                    }
                }
                catch (const CharacterDoesNotExist &) {
                    output << "Error caught\n";
                    continue;
                }
                catch (const IllegalItemType &) {
                    output << "Error caught\n";
                    continue;
                }
            }
            else {
                throw std::runtime_error("Unexpected command");
            }
        }
    }

    // Closing files

    input.close();
    output.close();
}

std::shared_ptr<Game> Game::currentGame()
{
    if (game == nullptr) {
        // Single instance is created, smart pointer is used
        game.reset(new Game());
    }
    return game;
}

void Game::destroyCharacter(std::shared_ptr<Character> ptr)
{
    characters.removeItem(ptr);
    output << ptr->getName() << " has died...\n";
    ptr.reset();
}

std::ofstream &Game::getOutput()
{
    return output;
}

// Setting the static variables

inline int Fighter::maxAllowedWeapons{3};

inline int Fighter::maxAllowedPotions{5};

inline int Archer::maxAllowedWeapons{2};

inline int Archer::maxAllowedPotions{3};

inline int Archer::maxAllowedSpells{2};

inline int Wizard::maxAllowedPotions{10};

inline int Wizard::maxAllowedSpells{10};

int main()
{

    // Start of game session
    auto game = Game::currentGame();
    game->startNewGame();
    return 0;
}
