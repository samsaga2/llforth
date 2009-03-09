#include "word.h"

WordInstance::WordInstance(Word *_word) : word(_word)
{
	assert(word != NULL);
}

Word *WordInstance::GetWord()
{
	return word;
}

void WordInstance::SetOutput(size_t index, llvm::Value *output)
{
	assert(index >= 0);
	if(outputs.size() <= index)
		outputs.resize(index + 1);
	outputs[index] = output;
}

llvm::Value *WordInstance::GetOutput(size_t index)
{
	assert(index >= 0 && index < GetOutputSize());
	return outputs[index];
}

void WordInstance::Compile()
{
	word->Execute(this);
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

