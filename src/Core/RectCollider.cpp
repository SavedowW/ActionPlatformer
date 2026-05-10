#include "RectCollider.h"

std::ostream& operator<< (std::ostream& out_, const Collider& cld_)
{
    out_ << "{ " << cld_.getLeftEdge() << ", " << cld_.getTopEdge() << ", " << cld_.getRightEdge() << ", " << cld_.getBottomEdge() << " }";
    return out_;
}
