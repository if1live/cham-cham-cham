﻿// Ŭnicode please 
#pragma once

#include "sequence.h"

class TitleSequence : public Sequence {
public:
	TitleSequence();
	~TitleSequence();

	virtual void update(int ms);
	virtual void preDraw();

private:
};
