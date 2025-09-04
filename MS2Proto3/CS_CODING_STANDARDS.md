## C# Coding Standards

To make our transpiler work, we must be very disciplined in how we write C# code.  This means both limiting what C# features we use, and standardize the way in which we write it.  Here we attempt to document all the restrictions.

## No shorthand integer types

Use the type names from `System` instead of the shorthand types.  For example, instead of `int`, write `Int32`.  (These compile to the same thing in C# but are more explicit.)

Including non-integer types, the full set of types you can use are: `Byte`, `SByte`, `Int16`, `UInt16`, `Int32`, `UInt32`, `Int64`, `UInt64`, `Char`, `Single`, `Double`, `Boolean`, `String`, `List<>`.

## Brace placement

- Always put an **opening brace at the end of a line**, never on a line by itself.

- Put a **close brace at the start of a line**.  It will usually be the *only* thing on that line, except for something like `} else {` or in a `switch` statement, `} break;`.

## Other limitations

- The `switch` statement may only be used with integer types (including enums).  For Strings or other custom types, use `if` statements instead.

## Capitalization

While not strictly required for the transpiler, in this project we follow C# capitalization conventions:

- All class names use PascalCase
- All public members (fields, methods, properties) also use PascalCase
- Private/protected fields use _underscoreCamelCase
- Parameters and local variables use camelCase

Example:
```
public class MySuperClass {
    public Int32 MyIntField;
    private String _secretStuff;
    
    public void DoThings(Int32 withValue, Boolean quickly) {
    	Int64 temp = withValue * 7;
    	if (!quickly) Utils::DoSlowThing();
    	DoQuickThing(temp);
    }
}
```

**BUT MAYBE:** It might make our transpiling much easier if we could distinguish fields (which in C++ require no parentheses to read) from properties (i.e. inline methods, which *do* require parentheses).  So we should consider altering the convention slightly, using camelCase instead of PascalCase for public fields.  Though that won't help with built-in types like String and List, so maybe it's not worth it.
    
    
## To be continued...

We'll add more restrictions here as we remember/implement them.
