#ifndef XR_SHORTCUT_H
#define XR_SHORTCUT_H

#pragma pack (push,1)
struct XRCORE_API xr_shortcut
{
	enum
	{
		flShift = 0x20,
		flCtrl = 0x40,
		flAlt = 0x80,
	};

	union
	{
		struct
		{
			u8 key;
			Flags8 ext;
		};

		u16 hotkey;
	};

	xr_shortcut(u8 k, BOOL a, BOOL c, BOOL s) : key(k)
	{
		ext.assign(u8((a ? flAlt : 0) | (c ? flCtrl : 0) | (s ? flShift : 0)));
	}

	xr_shortcut()
	{
		ext.zero();
		key = 0;
	}

	bool similar(const xr_shortcut& v) const { return ext.equal(v.ext) && (key == v.key); }
};
#pragma pack (pop)

#endif