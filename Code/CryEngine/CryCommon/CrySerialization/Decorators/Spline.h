// Copyright 2001-2017 Crytek GmbH / Crytek Group. All rights reserved. 

#pragma once

#include <CrySerialization/IArchive.h>
#include <CrySerialization/Decorators/ActionButton.h>
#include <CryEntitySystem/IEntitySystem.h>

namespace Serialization
{

struct ISpline
{
	ISpline() = default;
	ISpline(const ISpline& other)
		: m_points(other.m_points)
		, m_selectedPoint(other.m_selectedPoint) {}
	ISpline& operator=(const ISpline& other) = default;

	struct SPoint
	{
		SPoint() = default;
		SPoint(const Vec3& position)
			: pos(position)
			, back(position)
			, forw(position) {}

		Vec3  pos;
		Vec3  back;
		Vec3  forw;
		float angle = 0;
		float width = 0;
		bool  isDefaultWidth = true;

		void Serialize(Serialization::IArchive& ar)
		{
			ar(pos, "pos", "Position");
			ar(back, "back");
			ar(forw, "forw");
			ar(angle, "angle");
			ar(width, "width");
			ar(isDefaultWidth, "isDefaultWidth");
		}

		inline bool operator==(const SPoint &rhs) const
		{
			return pos == rhs.pos;
		}
	};

	virtual CryGUID GetOwnerInstanceGUID() const = 0;

	virtual void StartEditing() = 0;
	virtual void StopEditing() = 0;

	virtual Matrix34 GetWorldTransform() const = 0;
	
	virtual int GetMinPoints() const { return 2; }

	virtual void RemovePointByIndex(int index) = 0;

	virtual int InsertNewPoint(int index, const Vec3& position) = 0;
	virtual void UpdatePointByIndex(int index, const Vec3& newPosition) = 0;

	virtual void OnMovePoint() {}
	virtual void OnSelectedPointChanged() {}

	int GetPointCount() const { return m_points.size(); }
	const Vec3& GetPoint(int index) const { return m_points[index].pos; }
	int GetSelectedPoint() const { return m_selectedPoint; }
	bool HasSelectedPoint() const { return GetSelectedPoint() != -1; }

	void SelectPoint(int index)
	{
		if (m_selectedPoint == index)
			return;

		m_selectedPoint = index;
		OnSelectedPointChanged();
	}

	Vec3 GetSelectedPointWorldPosition() const
	{
		const Matrix34 wtm = GetWorldTransform();
		const int index = GetSelectedPoint();
		return wtm * GetPoint(index);
	}

	Vec3 GetBezierPos(int index, float t) const
	{
		float invt = 1.0f - t;
		return m_points[index].pos * (invt * invt * invt) +
			m_points[index].forw * (3 * t * invt * invt) +
			m_points[index + 1].back * (3 * t * t * invt) +
			m_points[index + 1].pos * (t * t * t);
	}

	Vec3 GetBezierTangent(int index, float t) const
	{
		float invt = 1.0f - t;
		Vec3 tan = -m_points[index].pos * (invt * invt)
			+ m_points[index].forw * (invt * (invt - 2 * t))
			+ m_points[index + 1].back * (t * (2 * invt - t))
			+ m_points[index + 1].pos * (t * t);

		if (!tan.IsZero())
			tan.Normalize();

		return tan;
	}

	float GetBezierSegmentLength(int index, float t = 1.f) const
	{
		const int kSteps = 32;
		float kn = t * kSteps + 1.0f;
		float fRet = 0.0f;

		Vec3 pos = GetBezierPos(index, 0.0f);
		for (float k = 1.0f; k <= kn; k += 1.0f)
		{
			Vec3 nextPos = GetBezierPos(index, t * k / kn);
			fRet += (nextPos - pos).GetLength();
			pos = nextPos;
		}
		return fRet;
	}

	Vec3 GetBezierNormal(int index, float t) const
	{
		float kof = 0.0f;
		int i = index;

		if (index >= GetPointCount() - 1)
		{
			kof = 1.0f;
			i = GetPointCount() - 2;
			if (i < 0)
				return Vec3(0, 0, 0);
		}

		const Matrix34 wtm = GetWorldTransform();
		Vec3 p0 = wtm.TransformPoint(GetBezierPos(i, t + 0.0001f + kof));
		Vec3 p1 = wtm.TransformPoint(GetBezierPos(i, t - 0.0001f + kof));

		Vec3 e = p0 - p1;
		Vec3 n = e.Cross(Vec3(0, 0, 1));

		if (n.x > 0.00001f || n.x < -0.00001f || n.y > 0.00001f || n.y < -0.00001f || n.z > 0.00001f || n.z < -0.00001f)
			n.Normalize();
		return n;
	}

	Vec3 GetLocalBezierNormal(int index, float t) const
	{
		float kof = t;
		int i = index;

		if (index >= GetPointCount() - 1)
		{
			kof = 1.0f + t;
			i = GetPointCount() - 2;
			if (i < 0)
				return Vec3(0, 0, 0);
		}

		Vec3 e = GetBezierTangent(i, kof);
		if (e.x < 0.00001f && e.x > -0.00001f && e.y < 0.00001f && e.y > -0.00001f && e.z < 0.00001f && e.z > -0.00001f)
			return Vec3(0, 0, 0);

		Vec3 n;

		float an1 = m_points[i].angle;
		float an2 = m_points[i + 1].angle;

		if (-0.00001f > an1 || an1 > 0.00001f || -0.00001f > an2 || an2 > 0.00001f)
		{
			float af = kof * 2 - 1.0f;
			float ed = 1.0f;
			if (af < 0.0f)
				ed = -1.0f;
			af = ed - af;
			af = af * af * af;
			af = ed - af;
			af = (af + 1.0f) / 2;
			float angle = ((1.0f - af) * an1 + af * an2) * 3.141593f / 180.0f;

			e.Normalize();
			n = Vec3(0, 0, 1).Cross(e);
			n = n.GetRotated(e, angle);
		}
		else
			n = Vec3(0, 0, 1).Cross(e);

		if (n.x > 0.00001f || n.x < -0.00001f || n.y > 0.00001f || n.y < -0.00001f || n.z > 0.00001f || n.z < -0.00001f)
			n.Normalize();
		return n;
	}

	void BezierCorrection(int index)
	{
		BezierAnglesCorrection(index - 1);
		BezierAnglesCorrection(index);
		BezierAnglesCorrection(index + 1);
		BezierAnglesCorrection(index - 2);
		BezierAnglesCorrection(index + 2);
	}

	void BezierAnglesCorrection(int index)
	{
		int maxindex = GetPointCount() - 1;
		if (index < 0 || index > maxindex)
		{
			return;
		}

		Vec3& p2 = m_points[index].pos;
		Vec3& back = m_points[index].back;
		Vec3& forw = m_points[index].forw;

		if (index == 0)
		{
			back = p2;
			if (maxindex == 1)
			{
				Vec3& p3 = m_points[index + 1].pos;
				forw = p2 + (p3 - p2) / 3;
			}
			else if (maxindex > 0)
			{
				Vec3& p3 = m_points[index + 1].pos;
				Vec3& pb3 = m_points[index + 1].back;

				float lenOsn = (pb3 - p2).GetLength();
				float lenb = (p3 - p2).GetLength();

				forw = p2 + (pb3 - p2) / (lenOsn / lenb * 3);
			}
		}

		if (index == maxindex)
		{
			forw = p2;
			if (index > 0)
			{
				Vec3& p1 = m_points[index - 1].pos;
				Vec3& pf1 = m_points[index - 1].forw;

				float lenOsn = (pf1 - p2).GetLength();
				float lenf = (p1 - p2).GetLength();

				if (lenOsn > 0.000001f && lenf > 0.000001f)
					back = p2 + (pf1 - p2) / (lenOsn / lenf * 3);
				else
					back = p2;
			}
		}

		if (1 <= index && index <= maxindex - 1)
		{
			Vec3& p1 = m_points[index - 1].pos;
			Vec3& p3 = m_points[index + 1].pos;

			float lenOsn = (p3 - p1).GetLength();
			float lenb = (p1 - p2).GetLength();
			float lenf = (p3 - p2).GetLength();

			back = p2 + (p1 - p3) * (lenb / lenOsn / 3);
			forw = p2 + (p3 - p1) * (lenf / lenOsn / 3);
		}
	}

	float GetSplineLength() const
	{
		float fRet = 0.f;
		for (int i = 0, numSegments = GetPointCount() - 1; i < numSegments; ++i)
		{
			fRet += GetBezierSegmentLength(i);
		}
		return fRet;
	}

	float GetPosByDistance(float distance, int& outIndex) const
	{
		float lenPos = 0.0f;
		int index = 0;
		float segmentLength = 0.0f;
		for (int numSegments = GetPointCount() - 1; index < numSegments; ++index)
		{
			segmentLength = GetBezierSegmentLength(index);
			if (lenPos + segmentLength > distance)
				break;
			lenPos += segmentLength;
		}

		outIndex = index;
		return segmentLength > 0.0f ? (distance - lenPos) / segmentLength : 0.0f;
	}

	void GetNearestEdge(const Vec3& raySrc, const Vec3& rayDir, int& p1, int& p2, float& distance, Vec3& intersectPoint) const
	{
		p1 = -1;
		p2 = -1;

		distance = std::numeric_limits<float>::max();
		const Lineseg rayLine(raySrc, raySrc + rayDir * 100000.0f);

		const Matrix34 wtm = GetWorldTransform();

		int maxPoint = GetPointCount();
		if (maxPoint-- >= GetMinPoints())
		{
			for (int i = 0, j = 1; i < maxPoint; ++i, ++j)
			{
				int kn = 6;
				for (int k = 0; k < kn; ++k)
				{
					const Vec3 pi = wtm.TransformPoint(GetBezierPos(i, float(k) / kn));
					const Vec3 pj = wtm.TransformPoint(GetBezierPos(i, float(k) / kn + 1.0f / kn));

					const Lineseg edgeLine(pi, pj);
					float tempIntersectionPoint;
					const float d = Distance::Lineseg_LinesegSq(rayLine, edgeLine, static_cast<float*>(nullptr), &tempIntersectionPoint);

					if (d < distance)
					{
						distance = d;
						p1 = i;
						p2 = j;
						intersectPoint = edgeLine.GetPoint(tempIntersectionPoint);
					}
				}
			}
		}

		distance = sqrt(distance);
	}

	int GetNearestPoint(const Vec3& raySrc, const Vec3& rayDir, float& distance) const
	{
		int index = -1;
		distance = std::numeric_limits<float>::max();
		const Lineseg rayLine(raySrc, raySrc + rayDir * 100000.0f);
		const Matrix34 wtm = GetWorldTransform();

		for (int i = 0, n = GetPointCount(); i < n; ++i)
		{
			float t;
			float d = Distance::Point_LinesegSq(wtm.TransformPoint(m_points[i].pos), rayLine, t);

			if (d < distance && d > 0)
			{
				distance = d;
				index = i;
			}
		}

		distance = sqrt(distance);

		return index;
	}

	void SerializeToFromDisk(Serialization::IArchive& ar)
	{
		ar(m_points, "points");
	}

protected:
	std::vector<SPoint> m_points;
	int m_selectedPoint = -1;
};

struct SplineButton final : public ISpline
{
	SplineButton(IEntityComponent* pComponent)
		: m_pComponent(pComponent) {}

	// ISplineEditor
	virtual CryGUID GetOwnerInstanceGUID() const override { return m_pComponent->GetEntity()->GetGuid(); }

	virtual void StartEditing() override { m_editing = true; }
	virtual void StopEditing() override { m_editing = false; }

	virtual Matrix34 GetWorldTransform() const override
	{
		return m_pComponent->GetWorldTransformMatrix();
	}

	virtual void RemovePointByIndex(int index) override
	{
		if ((index >= 0 || index < GetPointCount()) && GetPointCount() > GetMinPoints())
		{
			if (index == GetSelectedPoint())
			{
				SelectPoint(-1);
			}

			m_points.erase(m_points.begin() + index);
		}
	}

	virtual int InsertNewPoint(int index, const Vec3& position) override
	{
		if (index < 0 || index >= GetPointCount())
		{
			m_points.emplace_back(position);
			index = GetPointCount() - 1;
		}
		else
		{
			m_points.emplace(m_points.begin() + index, position);
		}

		BezierCorrection(index);
		SelectPoint(index);
		return index;
	}

	virtual void UpdatePointByIndex(int index, const Vec3& newPosition) override
	{
		m_points[index].pos = newPosition;
		BezierCorrection(index);
	}
	// ~ISplineEditor

	inline bool operator==(const SplineButton &rhs) const
	{
		if (m_points.size() != rhs.m_points.size())
		{
			return false;
		}

		return std::equal(m_points.begin(), m_points.end(), rhs.m_points.begin());
	}

	IEntityComponent* m_pComponent;
	bool m_editing = false;
};

inline bool Serialize(Serialization::IArchive& ar, SplineButton& button, const char* name, const char* label)
{
	if (ar.isEdit())
	{
		if (ar.openBlock(name, label))
		{
			ar(Serialization::SStruct::forEdit(static_cast<Serialization::ISpline&>(button)), "edit", "^Edit");

			ar.closeBlock();
			return true;
		}
	}

	button.SerializeToFromDisk(ar);
	return true;
}

}