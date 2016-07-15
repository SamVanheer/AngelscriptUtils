
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

int main( const string& in szString )
{
	Print( "foo\nbar\n" );
	Print( szString );
	
	g_HookManager.HookFunction( Hooks::Main, MainFunc );
	g_HookManager.HookFunction( Hooks::Main, @MainHook( Foo().Func ) );
	
	dictionary foo;
	
	foo.set( "bar", @MainHook( @Foo().Func ) );
	
	MainHook@ pFunc;
	
	foo.get( "bar", @pFunc );
	
	pFunc( "test\n" );
	
	Scheduler.SetTimeout( "Func", 5, "what's going on" );
	
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
	
	return 1;
}