#ifndef XR_RTOKEN_H
#define XR_RTOKEN_H

struct XRCORE_API xr_rtoken
{
	shared_str name;
	int id;

	xr_rtoken(LPCSTR _nm, int _id)
	{
		name = _nm;
		id = _id;
	}

public:
	void rename(LPCSTR _nm) { name = _nm; }
	bool equal(LPCSTR _nm) { return (0 == xr_strcmp(*name, _nm)); }
};

DEFINE_VECTOR(xr_rtoken, RTokenVec, RTokenVecIt);

#endif