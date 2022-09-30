/*
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
*/

enum E
{
	VAL = 0,
	VAL2
}

class Foo
{
	void Func(MyEvent@ args)
	{
		Print("method " + (args.ShouldHide ? "true" : "false"));
	}
}

funcdef void FuncPtr();

bool Function( int integer, Foo@ pFoo, E e, FuncPtr@ pFunc, const string& in szString, size_t size )
{
	Print( szString );
	
	return true;
}

void MainFunc(MyEvent@ args)
{
	Print( "hook called\n" );
}

class ScheduledFunc
{
	string szString;
	
	void Func()
	{
		Print( "Scheduler callback: " + szString + "\n" );
	}
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
	
	void SayHi()
	{
		Print("Hi, i'm the Lifetime object\n");
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
			
			Print( pMethod.GetDeclaration( includeObjectName: false, includeParamNames: true ) + "\n" );
		}
	}
}

void RunReflectionTests()
{
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

class HookEvent
{
	void Hook(MyEvent@ args)
	{
		Print( "HookEvent lookup works\n" );
		
		Event<MyEvent>(@g_EventSystem).Unsubscribe(@MyEventHandler(HookEvent().Hook));
	}
}

class EventTest
{
	void EventCallback(MyEvent@ args)
	{
		Print("Event callback\n");
		
		args.ShouldHide = true;
		
		//Test adding an removing an event handler during event execution
		EventTest obj;
		
		Event<MyEvent>(@g_EventSystem).Subscribe(@MyEventHandler(obj.EventCallback));
		Event<MyEvent>(@g_EventSystem).Unsubscribe(@MyEventHandler(obj.EventCallback));
	}
}

EventTest g_EventTest;

int main( const string& in szString )
{
	Print( "foo\nbar\n" );
	Print( szString );
	
	Event<MyEvent>(@g_EventSystem).Subscribe(@MyEventHandler(g_EventTest.EventCallback));
	
	Event<MyEvent>(@g_EventSystem).Subscribe(@MainFunc);
	Event<MyEvent>(@g_EventSystem).Subscribe(@MyEventHandler(Foo().Func));
	
	ScheduledFunc func;
	func.szString = "what's going on";
	Scheduler.Schedule( ScheduledCallback(@func.Func), 5 );
	
	//PrintReflection();
	
	RunReflectionTests();

	return 1;
}

//This function deliberately triggers a null pointer exception so the context result handler can log it.
void DoNullPointerException()
{
	dictionary@ pDict = null;
	
	Print( "checking null pointer\n" );
	Print( "Value: " + pDict.getKeys()[ 0 ] + "\n" );
}

//This function deliberately triggers a null pointer exception so the context result handler can log it. Object member function version to format it differently.
namespace Naaa
{
class NullAccess
{
	void TryAccess()
	{
		dictionary@ pDict = null;
	
		Print( "checking null pointer\n" );
		Print( "Value: " + pDict.getKeys()[ 0 ] + "\n" );
	}
}
}

void DoNullPointerException2()
{
	Naaa::NullAccess().TryAccess();
}

void TemplatedCallTest(int& out foo, int enumValue, MyEvent@ event, const string& in text)
{
	foo = 20;
	
	event.ShouldHide = true;
	
	Print("Value of foo is: " + formatInt(foo) + ", Text is: " + text + "\n");
}

void TestSuspendCall()
{
	Print("Calling SuspendTest()...\n");
	
	SuspendTest();
	
	Print("Resumed execution\n");
}
