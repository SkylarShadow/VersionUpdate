

#include "Abandonable/UnAbandonable.h"

FUnAbandonable::FUnAbandonable(const FSimpleDelegate &InThreadDelegate)
	:ThreadDelegate(InThreadDelegate)
{

}

void FUnAbandonable::DoWork()
{
	ThreadDelegate.ExecuteIfBound();
}