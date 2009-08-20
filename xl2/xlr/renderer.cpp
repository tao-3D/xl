// ****************************************************************************
//  renderer.cpp                    (C) 1992-2009 Christophe de Dinechin (ddd) 
//                                                                 XL2 project 
// ****************************************************************************
// 
//   File Description:
// 
//     Rendering of XL trees
// 
// 
// 
// 
// 
// 
// 
// 
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include "renderer.h"
#include <iostream>
#include <cctype>

XL_BEGIN

void Renderer::Indent(text t)
// ----------------------------------------------------------------------------
//   Emit the given text with proper indentation
// ----------------------------------------------------------------------------
{
    for (text::iterator i = t.begin(); i != t.end(); i++)
    {
        char c = *i;
        if (isspace(c))
            output << c;
        else
            output << need_space << c;
        need_space = "";
        if (c == '\n')
            for (uint i = 0; i < indent; i++)
                output << ' ';
    }
}


Tree *Renderer::Run(Tree *what)
// ----------------------------------------------------------------------------
//   Render the normalized trees
// ----------------------------------------------------------------------------
{
    if (!what)
    {
        output << need_space << "NULL";
    }
    else if (Integer *i = dynamic_cast<Integer *> (what))
    {
        output << need_space << i->value;
    }
    else if (Real *r = dynamic_cast<Real *> (what))
    {
        output << need_space << r->value;
    }
    else if (Text *t = dynamic_cast<Text *> (what))
    {
        output << need_space << t->Opening();
        need_space = "";
        Indent (t->value);
        output << t->Closing();
    }
    else if (Name *n = dynamic_cast<Name *> (what))
    {
        output << need_space << n->value;
    }
    else if (Block *b = dynamic_cast<Block *> (what))
    {
        static Block iblk(NULL);
        text open = b->Opening(), close = b->Closing();

        indent += tabsize;
        if (open == iblk.Opening())
            Indent("\n");
        else
            output << open;

        Run(b->child);
        
        indent -= tabsize;
        if (close == iblk.Closing())
            Indent("\n");
        else
            output << close;
    }
    else if (Prefix *p = dynamic_cast<Prefix *> (what))
    {
        Run(p->left);
        need_space = " ";
        Run(p->right);
    }
    else if (Infix *f = dynamic_cast<Infix *> (what))
    {
        uint count = f->list.size() - 1;
        for (tree_list::iterator c = f->list.begin(); c != f->list.end(); c++)
        {
            Run(*c);
            if (count--)
            {
                if (f->name == "\n" || f->name == "." || f->name == ",")
                    need_space = "";
                else
                    need_space = " ";
                Indent(f->name);
                if (f->name == "\n" || f->name == ".")
                    need_space = "";
                else
                    need_space = " ";
            }
        }
    }
    else
    {
        Tree *any = what->Normalize();
        if (any && any != what)
            Run(any);
        else
            output << "DENORM@" << (void *) what;
    }

    return what;
}


void Renderer::Render (std::ostream &out, Tree *t)
// ----------------------------------------------------------------------------
//   Render a tree
// ----------------------------------------------------------------------------
{
    Renderer renderer(out);
    renderer.Run(t);
}


XL_END

void debug(XL::Tree *tree)
// ----------------------------------------------------------------------------
//    Emit for debugging purpose
// ----------------------------------------------------------------------------
{
    std::cout << tree << "\n";
}
