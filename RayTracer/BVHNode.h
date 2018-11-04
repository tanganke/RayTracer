#pragma once

#include "VisibleObject.h"
#include "AxisAlignedBoundingBox.h"
#include "Random.h"

#include <algorithm>

namespace BVH
{

	class BVHNode : public Objects::VisibleObject
	{
	public:
		BVHNode() {};

		BVHNode(std::vector<std::shared_ptr<Objects::VisibleObject>>::iterator start, std::vector<std::shared_ptr<Objects::VisibleObject>>::iterator end, int dir = 0)
		{
			const int size = end - start;
			if (0 == size) return;

			if (0 == dir)
			{
				std::sort(start, end, [](const auto& o1, const auto& o2)
				{
					AxisAlignedBoundingBox b1, b2;

					o1->BoundingBox(b1);
					o2->BoundingBox(b2);

					return b1.min().X < b2.min().X;
				});
			}
			else if (1 == dir)
			{
				std::sort(start, end, [](const auto& o1, const auto& o2)
				{
					AxisAlignedBoundingBox b1, b2;

					o1->BoundingBox(b1);
					o2->BoundingBox(b2);

					return b1.min().Y < b2.min().Y;
				});
			}
			else
			{
				std::sort(start, end, [](const auto& o1, const auto& o2)
				{
					AxisAlignedBoundingBox b1, b2;

					o1->BoundingBox(b1);
					o2->BoundingBox(b2);

					return b1.min().Z < b2.min().Z;
				});
			}

			if (1 == size)
			{
				child1 = *start;
				child1->BoundingBox(boundingBox);
			}
			else if (2 == size)
			{
				child1 = *start;
				child2 = *(start + 1);

				AxisAlignedBoundingBox b1, b2;
				child1->BoundingBox(b1);
				child2->BoundingBox(b2);
				boundingBox = AxisAlignedBoundingBox::EnclosingBox(b1, b2);
			}
			else
			{
				if (++dir > 2) dir = 0;

				const int mIndex = size / 2;
				child1 = std::dynamic_pointer_cast<Objects::VisibleObject>(std::make_shared<BVHNode>(start, start + mIndex, dir));
				child2 = std::dynamic_pointer_cast<Objects::VisibleObject>(std::make_shared<BVHNode>(start + mIndex, end, dir));

				AxisAlignedBoundingBox b1, b2;
				child1->BoundingBox(b1);
				child2->BoundingBox(b2);
				boundingBox = AxisAlignedBoundingBox::EnclosingBox(b1, b2);
			}
		}

		virtual bool Hit(const Ray& ray, PointInfo& info, double minr, double maxr, unsigned rcount, Random& random) const override
		{
			if (boundingBox.Intersects(ray, minr, maxr))
			{
				PointInfo info1, info2;

				const bool hit1 = child1->Hit(ray, info1, minr, maxr, rcount, random);
				const bool hit2 = child2 ? child2->Hit(ray, info2, minr, maxr, rcount, random) : false;

				if (hit1 && hit2)
				{
					if (info1.distance < info2.distance)
						info = info1;
					else info = info2;

					return true;
				}
				else if (hit1)
				{
					info = info1;
					return true;
				}
				else if (hit2)
				{
					info = info2;
					return true;
				}
			}

			return false;
		}

		virtual bool BoundingBox(AxisAlignedBoundingBox& box) override
		{
			box = boundingBox;

			return true;
		}


		virtual void Translate(const Vector3D<double>& t) override
		{
			boundingBox.Translate(t);

			child1->Translate(t);
			if (child2) child2->Translate(t);
		}

		virtual void RotateAround(const Vector3D<double>& v, double angle)
		{
			child1->RotateAround(v, angle);
			child1->BoundingBox(boundingBox);

			if (child2)
			{
				AxisAlignedBoundingBox b;
				child2->RotateAround(v, angle);
				child2->BoundingBox(b);

				boundingBox = AxisAlignedBoundingBox::EnclosingBox(boundingBox, b);
			}
		}


		virtual void Scale(double s) override
		{
			boundingBox.Scale(s);
			child1->Scale(s);
			if (child2) child2->Scale(s);
		}



		AxisAlignedBoundingBox boundingBox;

		// only leaves are something else than BVHNode
		std::shared_ptr<VisibleObject> child1;
		std::shared_ptr<VisibleObject> child2;
	};


}
