#pragma once

class Resource {
public:
	virtual ~Resource() = default;

	virtual void destroy() = 0;
};