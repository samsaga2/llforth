#include "word.h"

WordInstance::WordInstance(Word *_word) : word(_word)
{
	assert(word != NULL);

	inputs.resize(word->GetInputSize());

	if(word->GetOutputSize() == 0)
		outputs = NULL;
	else
		outputs = new llvm::Value *[word->GetOutputSize()];
}

WordInstance::~WordInstance()
{
	if(outputs != NULL)
		delete outputs;
}

Word *WordInstance::GetWord()
{
	return word;
}

void WordInstance::SetInput(size_t index, WordIndex *word_index)
{
	assert(word != NULL);
	assert(index >= 0 && index < word->GetInputSize());
	inputs[index] = word_index;
}

WordIndex *WordInstance::GetInput(size_t index)
{
	assert(index >= 0 && index < word->GetInputSize());
	return inputs[index];
}

void WordInstance::SetOutput(size_t index, llvm::Value *output)
{
	assert(index >= index && index < word->GetOutputSize());
	outputs[index] = output;
}

llvm::Value *WordInstance::GetOutput(size_t index)
{
	assert(index >= 0 && index < word->GetOutputSize());
	return outputs[index];
}

void WordInstance::Compile(Engine *e)
{
	word->Compile(e, this);
}

WordIndex::WordIndex(WordInstance *word_instance, size_t index)
{
	this->word_instance = word_instance;
	this->index = index;
}

WordInstance *WordIndex::GetWordInstance()
{
	return word_instance;
}

size_t WordIndex::GetIndex()
{
	return index;
}

llvm::Value *WordIndex::GetOutput()
{
	return word_instance->GetOutput(index);
}

