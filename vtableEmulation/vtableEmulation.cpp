// vtableEmulation.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <typeinfo>

namespace gamestuff
{
	///<summary>GameObject is a pure virtual class, you can't instantiate objects of this type
	///</summary>
	class GameObject
	{
	public:
		GameObject( ) {};
		virtual ~GameObject( ) = 0;
		virtual void fly( void ) = 0;
		///<summary>out takes a std::ostream and writes the object to the stream provided.</summary>
		virtual std::ostream& out( std::ostream& stream ) const = 0;

		GameObject( const GameObject& ) = default;
		GameObject& operator=( const GameObject& ) = default;
	private:
		
	};

	GameObject::~GameObject( ) {
		std::cout << "Game object destructor\n";
	}

	///<summary>overload helper for operator to stream game objects</summary>
	std::ostream& operator<<(std::ostream& stream, const GameObject& object){
		return object.out( stream );
	}

	class Spaceship : public GameObject
	{
	private:
		double _maxSpeed;
		std::string _name;

	public:
		Spaceship( const std::string& name, double maxSpeed ):_name( name )
			,_maxSpeed( maxSpeed ) 
		{
			std::cout << "Spaceship constructed\n";
		}
		~Spaceship( ) {
			std::cout << "Spaceship destructor\n";
		}

		std::ostream& out( std::ostream& stream ) const override {
			return stream << "Spaceship: " << _name;
		}

		void fly( void ) override {
			std::cout << "Spaceship: " << _name.c_str() << " flying at " << _maxSpeed << "mph.\n";
		}
	};

	class Spacestation : public GameObject
	{
	private:
		double _maxSpeed;
		std::string _name;

		
	public:
		Spacestation( const std::string& name, double maxSpeed ) :_name(name)
		,_maxSpeed(maxSpeed) 
		{
			std::cout << "Spacestation constructed\n";
		}

		~Spacestation( ) {
			std::cout << "Spacestation destructor\n";
		}

		std::ostream& out( std::ostream& stream ) const override {
			return stream << "Spacestation: " << _name;
		}

		void fly( void ) override {
			std::cout << "Spacestation: " << _name << " orbiting at " << _maxSpeed << "mph.\n";
		}
	};

	class Asteroid : public GameObject
	{
	private:
		double _maxSpeed;
		std::string _name;

	public:
		Asteroid( const std::string& name, double maxSpeed ) :_name( name )
			, _maxSpeed( maxSpeed )
		{
			std::cout << "Asteroid constructed\n";
		}
		~Asteroid( ) {
			std::cout << "Asteroid destructor\n";
		}

		// factory function
		//Spaceship* makeSpaceship( const std::string& name, double _maxSpeed ) {
		//	return new Spaceship( "Enterprise", 1000.00 );
		//}

		std::ostream& out( std::ostream& stream ) const override {
			return stream << "Asteroid: " << _name;
		}

		void fly( void ) override {
			std::cout << "Asteroid: " << _name.c_str( ) << " flying at " << _maxSpeed << "mph.\n";
		}
	};

	using collisionFunctionPtr = void (*)( const GameObject& lhs, const GameObject& rhs );

	using gameObjectNames = std::pair<std::string, std::string>;
	using collisionMap = std::map< gameObjectNames, collisionFunctionPtr >;


	collisionMap& getCollisionMap( void ) {
		static std::unique_ptr<collisionMap> theMap = std::make_unique<collisionMap>( );
		return *theMap;
	}

	class RegisterCollisionMap
	{
	private:

	public:
		
		static void RegisterCollisionMapFunction(const type_info& obj1, const type_info& obj2,
			collisionFunctionPtr ptrFunction, bool isSymmetric = true ) {
			std::cout << "Registering function for collision between " <<
				obj1.name() << " and " << obj2.name() << "\n";

			using collisionMapValue = 
				std::pair< const gameObjectNames&, collisionFunctionPtr >;

			const gameObjectNames gameObjects = gameObjectNames( obj1.name( ),
				obj2.name( ) );
			
			getCollisionMap( ).insert( collisionMapValue( gameObjects, ptrFunction ) );

			if ( isSymmetric ) {
				const gameObjectNames gameObjectsSwapped = gameObjectNames( obj2.name( ),
					obj1.name( ) );
				getCollisionMap( ).insert( collisionMapValue( gameObjectsSwapped, ptrFunction ) );

			 } // symetric

		}
	};

} // gamestuff

///<summary>looks up the function pointer based on the string type ids of the objects.</summary>
///<returns>collisionFuctionPtr - a function pointer</returns>
gamestuff::collisionFunctionPtr findCollisionFunction( const std::string& name1, const std::string& name2 )
{
	auto theMap = gamestuff::getCollisionMap( );
	auto iter = theMap.find( gamestuff::gameObjectNames( name1, name2) );
	
	if ( theMap.end( ) == iter ) {
		return nullptr;
	}

	return (*iter).second;
}

// this function searches our map of objects for the correct collision function
///<summary>processCollision takes two seperate game objects and calls the correct
///function for their derived types. This method is the main hub of our vtable
///emulation. It takes the types of both objects and looks for an appropriate
///virtual function. If one is found it's called, else we simply log that there is
///no appropriate function for the game objects passed to the method.</summary>
///<returns>void</returns>
void processCollision( gamestuff::GameObject& obj1, gamestuff::GameObject& obj2 )
{
	// find the collision function pointer function
	auto collisionFncPtr = findCollisionFunction( typeid(obj1).name(), 
		typeid(obj2).name() );
	if ( nullptr == collisionFncPtr ) {
		std::cout << "No collision function pointer exists\n";
		return;
	}

	// call the function pointer on the two objects
	collisionFncPtr( obj1, obj2 );

}

///<summary>
/// Helper class CollisionRegister provides a location for 
/// registering two GameObject derivatives. The constructor takes both
/// classes represented by their type info, the collision function ptr and
/// if the function is symmetrical, i.e. a collision between a spaceship
/// and an asteroid is the same as a collision between an asteroid and a
/// spaceship
///</summary>
class CollisionRegister
{
public:
	CollisionRegister(const type_info& obj1, const type_info& obj2,
		gamestuff::collisionFunctionPtr pfnc,
		bool symmetric = true)
	{
		gamestuff::RegisterCollisionMap::RegisterCollisionMapFunction( obj1,
			obj2,
			pfnc,
			symmetric );
	}

	~CollisionRegister( ) {};
	CollisionRegister( const CollisionRegister& ) = delete;
	CollisionRegister& operator=( const CollisionRegister& ) = delete;
};

void processSpaceshipSpaceshipCollision( const gamestuff::GameObject& ship1, 
	const gamestuff::GameObject& ship2 )
{
	std::cout << "Collision between 2 spaceships: " << ship1 << " and " << ship2 << "\n";
}

void processSpaceshipSpacestaionCollision( const gamestuff::GameObject& ship, 
	const gamestuff::GameObject& station )
{
	std::cout << "Collision between spaceship and spacestation: " << ship << " and " << station << "\n";
}

void processAsteroidSpacestaionCollision( const gamestuff::GameObject& asteroid, 
	const gamestuff::GameObject& station )
{
	std::cout << "Collision between asteroid and spacestation: " << asteroid << " and " << station << "\n";
}

void processSpaceshipAsteroidCollision( const gamestuff::GameObject& ship, 
	const gamestuff::GameObject& asteroid )
{
	std::cout << "Collision between spaceship and asteroid: " << ship << " and " << asteroid << "\n";
}

CollisionRegister spaceships( typeid( gamestuff::Spaceship ),
	typeid( gamestuff::Spaceship ), processSpaceshipSpaceshipCollision, false );

CollisionRegister spaceshipAsteroid( typeid(gamestuff::Spaceship),
	typeid(gamestuff::Asteroid), processSpaceshipAsteroidCollision, true );

CollisionRegister spaceshipSpacestation( typeid(gamestuff::Spaceship),
	typeid(gamestuff::Spacestation), processSpaceshipSpacestaionCollision, true );

CollisionRegister spacestationAsteroid( typeid(gamestuff::Asteroid),
	typeid(gamestuff::Spacestation), processAsteroidSpacestaionCollision, true );


int main( )
{
	std::cout << "Hello World!\n";

	std::vector<gamestuff::GameObject*> gameObjects;
	gameObjects.reserve( 3 );
	gameObjects.emplace_back( new gamestuff::Spaceship( "Enterprise", 1000 ) );
	gameObjects.emplace_back( new gamestuff::Spaceship( "Millenium Falcon", 2000 ) );
	gameObjects.emplace_back( new gamestuff::Spacestation( "Deep space 9", 2 ) );
	gameObjects.emplace_back( new gamestuff::Asteroid( "Hayley's Comet", 150 ) );

	for ( auto& go : gameObjects ) {
		std::cout << (*go) << "\n";
	}

	processCollision( *(gameObjects[0]), *(gameObjects[1]) );
	processCollision( *(gameObjects[0]), *(gameObjects[2]) );
	processCollision( *(gameObjects[0]), *(gameObjects[3]) );
	processCollision( *(gameObjects[2]), *(gameObjects[1]) );
	processCollision( *(gameObjects[2]), *(gameObjects[3]) );
	processCollision( *(gameObjects[3]), *(gameObjects[2]) );
	processCollision( *(gameObjects[3]), *(gameObjects[0]) );

	std::cout << "the end..\n";

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
