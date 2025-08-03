#include "dxUISequenceVideoItem.h"
#include "R_Backend_Runtime.h"

dxUISequenceVideoItem::dxUISequenceVideoItem()
{
	m_texture = 0;
}

void dxUISequenceVideoItem::Copy(IUISequenceVideoItem& _in)
{
	*this = *((dxUISequenceVideoItem*)&_in);
}

void dxUISequenceVideoItem::CaptureTexture()
{
	m_texture = RCache.get_ActiveTexture(0);
}
