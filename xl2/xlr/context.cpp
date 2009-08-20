// ****************************************************************************
//  context.cpp                     (C) 1992-2003 Christophe de Dinechin (ddd) 
//                                                                 XL2 project 
// ****************************************************************************
// 
//   File Description:
// 
//     Description of an execution context
// 
// 
// 
// 
// 
// 
// 
// 
// ****************************************************************************
// This document is confidential.
// Do not redistribute without written permission
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include "context.h"
#include "tree.h"
#include <iostream>
#include <cstdlib>

XL_BEGIN

// ============================================================================
// 
//   Garbage collection
// 
// ============================================================================

struct GCAction : Action
// ----------------------------------------------------------------------------
//   An action used for garbage collection
// ----------------------------------------------------------------------------
{
    GCAction (): alive() {}
    ~GCAction () {}

    Tree *Run(Tree *what)
    {
        typedef std::pair<active_set::iterator, bool> inserted;
        inserted ins = alive.insert(what);
        if (ins.second)
            return what->Mark(*this);
        return what;
    }
    active_set  alive;
};


ulong Context::gc_increment = 200;
ulong Context::gc_growth_percent = 110;

void Context::CollectGarbage ()
// ----------------------------------------------------------------------------
//   Mark all active trees
// ----------------------------------------------------------------------------
{
    if (active.size() > gc_threshold)
    {
        GCAction gc;
        active_set::iterator i;
        evaluation_stack::iterator s;

        // Mark roots and stack
        for (i = roots.begin(); i != roots.end(); i++)
            (*i)->Mark(gc);
        for (s = stack.begin(); s != stack.end(); s++)
            (*s)->Mark(gc);

        // Then delete all trees in active set that are no longer referenced
        for (i = active.begin(); i != active.end(); i++)
        {
            if (!gc.alive.count(*i))
                delete *i;
        }
        active = gc.alive;
        gc_threshold = active.size() * gc_growth_percent / 100 + gc_increment;
    }
}


Tree *Context::Run (Tree *what)
// ----------------------------------------------------------------------------
//    Execute a tree in the current context and put its value on TOS
// ----------------------------------------------------------------------------
{
    Push(what);
    CollectGarbage();
    return what->Run(this);
}


Tree * Context::Error(text message, Tree *args)
// ----------------------------------------------------------------------------
//   Execute the innermost error handler
// ----------------------------------------------------------------------------
{
    Tree *handler = ErrorHandler();
    if (handler)
    {
        Tree *info = new XL::Text (message);
        if (args)
            info = new XL::Prefix(info, args, args->Position());
        Push(info);
        return handler->Run(this);
    }

    // No handler: terminate
    std::cerr << "Error: No error handler\n" << "Message: " << message << "\n";
    std::exit(1);
}


uint Context::Push(Tree *tos)
// ----------------------------------------------------------------------------
//   Put the tree as top-of-stack
// ----------------------------------------------------------------------------
{
    if (tos)
    {
        active.insert(tos);
        stack.push_back(tos);
    }
    return stack.size();
}


Tree *Context::Pop(void)
// ----------------------------------------------------------------------------
//   Get the top of stack
// ----------------------------------------------------------------------------
{
    if (stack.empty())
        return Error("Execution stack is empty");
    Tree *tos = stack.back();
    stack.pop_back();
    return tos;
}


Tree *Context::Peek (uint depth)
// ----------------------------------------------------------------------------
//   Peeks at the N-th element in the stack
// ----------------------------------------------------------------------------
{
    evaluation_stack::size_type sz = stack.size();
    if (depth == 0 || depth > sz)
        return Error("Peeking at non-existent stack element");
    return stack.at(sz - depth);
}



// ============================================================================
// 
//    Properties and symbol management
// 
// ============================================================================

Tree *Context::ErrorHandler()
// ----------------------------------------------------------------------------
//    Return the innermost error handler
// ----------------------------------------------------------------------------
{
    for (Context *c = this; c; c = c->Parent())
        if (c->error_handler)
            return c->error_handler;
    return NULL;
}


Tree *Context::Name(text name, bool deep)
// ----------------------------------------------------------------------------
//   Lookup a name in the symbol table
// ----------------------------------------------------------------------------
{
    for (Context *c = this; c; c = c->Parent())
    {
        if (c->name_symbols.count(name))
            return c->name_symbols[name];
        if (!deep)
            break;
    }
    return NULL;
}


Tree *Context::Prefix(text name, bool deep)
// ----------------------------------------------------------------------------
//   Lookup an infix operator in the symbol table
// ----------------------------------------------------------------------------
{
    for (Context *c = this; c; c = c->Parent())
    {
        if (c->prefix_symbols.count(name))
            return c->prefix_symbols[name];
        if (!deep)
            break;
    }
    return NULL;
}


Tree *Context::Postfix(text name, bool deep)
// ----------------------------------------------------------------------------
//   Lookup an infix operator in the symbol table
// ----------------------------------------------------------------------------
{
    for (Context *c = this; c; c = c->Parent())
    {
        if (c->postfix_symbols.count(name))
            return c->postfix_symbols[name];
        if (!deep)
            break;
    }
    return NULL;
}


Tree *Context::Block(text name, bool deep)
// ----------------------------------------------------------------------------
//   Lookup an infix operator in the symbol table
// ----------------------------------------------------------------------------
{
    for (Context *c = this; c; c = c->Parent())
    {
        if (c->block_symbols.count(name))
            return c->block_symbols[name];
        if (!deep)
            break;
    }
    return NULL;
}


Tree *Context::Infix(text name, bool deep)
// ----------------------------------------------------------------------------
//   Lookup an infix operator in the symbol table
// ----------------------------------------------------------------------------
{
    for (Context *c = this; c; c = c->Parent())
    {
        if (c->infix_symbols.count(name))
            return c->infix_symbols[name];
        if (!deep)
            break;
    }
    return NULL;
}


XL_END