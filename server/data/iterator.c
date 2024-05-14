# include "../include/chess.h"

inherit LIB_ITERATOROBJ;

private object owner;
private int    index, brand;

# define SECURE()	if(previous_object() != owner) error("illegal call");

mixed get_next()		{ error("deprecated, use next()"); }
mixed get_previous()	{ error("deprecated, use previous()"); }


static void create(varargs int clone)
{
#if 0
	owner = previous_object(CREATE_PREV_OBJ);
#endif
	owner = previous_object(0);
	index = -1;
}


/* Use to distinguish between multiple iterators in the same program */
void set_brand(int val)
{
	SECURE();
	brand = val;
}


/* Use to distinguish between multiple iterators in the same program */
int get_brand()
{
	return brand;
}


int index()
{
	return index;
}


mixed next()
{
	return (index < (owner->Iter_size()-1)) ? owner->Iter_get(++index) : nil;
}


mixed previous()
{
	return (index > 0) ? owner->Iter_get(--index) : nil;
}


int has_next()
{
	return (index < owner->Iter_size()-1) ? TRUE : FALSE;
}


int has_previous()
{
	return (index > 0) ? TRUE : FALSE;
}


int next_index()
{
	return has_next() ? (index + 1) : nil;
}


int previous_index()
{
	return has_previous() ? (index - 1) : nil;
}


void set(mixed val)
{
	owner->Iter_set(index, val);
}


void remove()
{
	error("TODO");
}


void add()
{
	error("TODO");
}

