
class CEntity : CScriptBaseEntity
{
	void Spawn()
	{
		Print( "Spawned\n" );
	}
	
	int ScheduleOfType( const string& in szName )
	{
		return BaseClass.ScheduleOfType( szName ) + 2;
	}
}

enum E
{
	VAL = 0,
	VAL2
}

class Foo
{
	HookReturnCode Func( const string& in szString )
	{
		Print( "method " + szString );
		return HOOK_CONTINUE;
	}
}

funcdef void FuncPtr();

bool Function( int integer, Foo@ pFoo, E e, FuncPtr@ pFunc, const string& in szString, size_t size )
{
	Print( szString );
	
	return true;
}

HookReturnCode MainFunc( const string& in szString )
{
	Print( "hook called\n" );

	return HOOK_CONTINUE;
}

void Func( const string& in szString )
{
	Print( szString + "\n" );
}

void NoArgs()
{
	Print( "No arguments works\n" );
}

class Lifetime
{
	Lifetime()
	{
		Print( "Lifetime constructed\n" );
	}
	
	~Lifetime()
	{
		Print( "Lifetime destroyed\n" );
	}
}

Lifetime@ GetLifetime()
{
	return @Lifetime();
}

void PrintReflection()
{
	/*
	*	Let's print out all global functions registered by the program.
	*/
	for( uint uiIndex = 0; uiIndex < Reflect::Engine.GetGlobalFunctionCount(); ++uiIndex )
	{
		Reflect::Function@ pFunction = Reflect::Engine.GetGlobalFunctionByIndex( uiIndex );
		
		Print( pFunction.GetNamespace() + "::" + pFunction.GetName() + "\n" );
	}
	
	/*
	*	Let's print out all object types and all of their method registered by the program.
	*/
	for( uint uiIndex = 0; uiIndex < Reflect::Engine.GetObjectTypeCount(); ++uiIndex )
	{
		Reflect::TypeInfo@ pType = Reflect::Engine.GetObjectTypeByIndex( uiIndex );
		
		Print( pType.GetNamespace() + "::" + pType.GetName() + "\n" );
		
		for( uint uiIndex2 = 0; uiIndex2 < pType.GetMethodCount(); ++uiIndex2 )
		{
			Reflect::Method@ pMethod = pType.GetMethodByIndex( uiIndex2 );
			
			Print( pMethod.GetDeclaration( bIncludeObjectName: false, bIncludeParamNames: true ) + "\n" );
		}
	}
}

namespace Foo
{
int Bar()
{
	return 0;
}

class Baz
{
}
}

int main( const string& in szString )
{
	Print( "foo\nbar\n" );
	Print( szString );
	
	Events::Main.Hook( MainFunc );
	Events::Main.Hook( @MainHook( Foo().Func ) );
	
	CEvent@ pEvent = g_EventManager.FindEventByName( "Main" );
	
	Print( "Event was found: " + ( pEvent !is null ? "yes" : "no" ) + "\n" );
	
	dictionary foo;
	
	foo.set( "bar", @MainHook( @Foo().Func ) );
	
	MainHook@ pFunc;
	
	foo.get( "bar", @pFunc );
	
	pFunc( "test\n" );
	
	Scheduler.SetTimeout( "Func", 5, "what's going on" );
	
	//PrintReflection();
	
	//Find a function in a namespace (Module).
	Reflect::Function@ pFunction = Reflect::Module.FindGlobalFunction( "Foo::Bar" );
	
	Print( "(Module) Found function by name: " + ( ( pFunction !is null ) ? "yes" : "no" ) + "\n" );
	
	@pFunction = Reflect::Module.FindGlobalFunction( "int Foo::Bar()", true );
	
	Print( "(Module) Found function by decl: " + ( ( pFunction !is null ) ? "yes" : "no" ) + "\n" );
	
	//Test for nonexistent.
	@pFunction = Reflect::Module.FindGlobalFunction( "Foo2::Bar" );
	
	Print( "(Module) Didn't find function by name: " + ( ( pFunction is null ) ? "yes" : "no" ) + "\n" );
	
	@pFunction = Reflect::Module.FindGlobalFunction( "int Foo2::Bar()", true );
	
	Print( "(Module) Didn't find function by decl: " + ( ( pFunction is null ) ? "yes" : "no" ) + "\n" );
	
	//Find a type in a namespace (Module).
	Reflect::TypeInfo@ pType = Reflect::Module.FindTypeInfo( "Foo::Baz" );
	
	Print( "(Module) Found type info by name: " + ( ( pType !is null ) ? "yes" : "no" ) + "\n" );
	
	@pType = Reflect::Module.FindTypeInfo( "Foo::Baz", true );
	
	Print( "(Module) Found type info by decl: " + ( ( pType !is null ) ? "yes" : "no" ) + "\n" );
	
	//Test for nonexistent.
	@pType = Reflect::Module.FindTypeInfo( "Foo2::Baz" );
	
	Print( "(Module) Didn't find type info by name: " + ( ( pType is null ) ? "yes" : "no" ) + "\n" );
	
	@pType = Reflect::Module.FindTypeInfo( "Foo2::Baz", true );
	
	Print( "(Module) Didn't find type info by decl: " + ( ( pType is null ) ? "yes" : "no" ) + "\n" );
	
	//Find a function in a namespace (Engine).
	@pFunction = Reflect::Engine.FindGlobalFunction( "NS::NSTest" );
	
	Print( "(Engine) Found function by name: " + ( ( pFunction !is null ) ? "yes" : "no" ) + "\n" );
	
	@pFunction = Reflect::Engine.FindGlobalFunction( "int NS::NSTest()", true );
	
	Print( "(Engine) Found function by decl: " + ( ( pFunction !is null ) ? "yes" : "no" ) + "\n" );
	
	//Test for nonexistent.
	@pFunction = Reflect::Engine.FindGlobalFunction( "Foo2::Bar" );
	
	Print( "(Engine) Didn't find function by name: " + ( ( pFunction is null ) ? "yes" : "no" ) + "\n" );
	
	@pFunction = Reflect::Engine.FindGlobalFunction( "int Foo2::Bar()", true );
	
	Print( "(Engine) Didn't find function by decl: " + ( ( pFunction is null ) ? "yes" : "no" ) + "\n" );
	
	//Find a type in a namespace (Engine).
	@pType = Reflect::Engine.FindTypeInfo( "Reflect::TypeInfo" );
	
	Print( "(Engine) Found type info by name: " + ( ( pType !is null ) ? "yes" : "no" ) + "\n" );
	
	@pType = Reflect::Engine.FindTypeInfo( "Reflect::TypeInfo", true );
	
	Print( "(Engine) Found type info by decl: " + ( ( pType !is null ) ? "yes" : "no" ) + "\n" );
	
	//Test for nonexistent.
	@pType = Reflect::Engine.FindTypeInfo( "Foo2::Baz" );
	
	Print( "(Engine) Didn't find type info by name: " + ( ( pType is null ) ? "yes" : "no" ) + "\n" );
	
	@pType = Reflect::Engine.FindTypeInfo( "Foo2::Baz", true );
	
	Print( "(Engine) Didn't find type info by decl: " + ( ( pType is null ) ? "yes" : "no" ) + "\n" );
	
	return 1;
}