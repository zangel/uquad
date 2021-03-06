#include "cvd/runnable_batch.h"

namespace CVD
{
RunnableBatch::RunnableBatch(unsigned int p)
:joined(0),parallelism(0)
{
}

void RunnableBatch::join()
{
	joined = 1;
}

void RunnableBatch::schedule(STD::shared_ptr<Runnable> r)
{
	r->run();
}

RunnableBatch::~RunnableBatch()
{
}
}
