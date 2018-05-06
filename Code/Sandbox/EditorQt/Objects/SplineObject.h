// Copyright 2001-2018 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CrySerialization/Decorators/Spline.h>
#include <Gizmos/ITransformManipulator.h>
#include <Viewport.h>

const float kSplinePointSelectionRadius = 0.8f;

//////////////////////////////////////////////////////////////////////////
class CSplineObject : public CBaseObject, public Serialization::ISpline
{
protected:
	CSplineObject();

public:
	virtual void  SetPoint(int index, const Vec3& pos);
	int           InsertPoint(int index, const Vec3& point);
	void          RemovePoint(int index);
	
	float         GetPointAngle() const;
	void          SetPointAngle(float angle);
	float         GetPointWidth() const;
	void          SetPointWidth(float width);
	bool          IsPointDefaultWidth() const;
	void          PointDafaultWidthIs(bool isDefault);

	void          SetEditMode(bool isEditMode) { m_isEditMode = isEditMode; }

	void          SetMergeIndex(int index)     { m_mergeIndex = index; }
	void          ReverseShape();
	void          Split(int index, const Vec3& point);
	void          Merge(CSplineObject* pSpline);

	void          CalcBBox();
	virtual float CreationZOffset() const { return 0.1f; }

	virtual void  OnUpdate()                  {}
	virtual void  SetLayerId(uint16 nLayerId) {}
	virtual void  SetPhysics(bool isPhysics)  {}
	virtual float GetAngleRange() const       { return 180.0f; }

	virtual void  OnContextMenu(CPopupMenuItem* menu);

protected:
	DECLARE_DYNAMIC(CSplineObject);
	void          OnUpdateUI();

	void          DrawJoints(DisplayContext& dc);
	bool          RayToLineDistance(const Vec3& rayLineP1, const Vec3& rayLineP2, const Vec3& pi, const Vec3& pj, float& distance, Vec3& intPnt);

	virtual int   GetMaxPoints() const { return 1000; }
	virtual float GetWidth() const     { return 0.0f; }
	virtual float GetStepSize() const  { return 1.0f; }

	// from CBaseObject
	bool         Init(CBaseObject* prev, const string& file) override;
	void         Done() override;

	void CreateInspectorWidgets(CInspectorWidgetCreator& creator) override;

	void         GetBoundBox(AABB& box) override;
	void         GetLocalBounds(AABB& box) override;
	void         Display(CObjectRenderHelper& objRenderHelper) override;
	bool         HitTest(HitContext& hc) override;
	bool         HitTestRect(HitContext& hc) override;
	void         Serialize(CObjectArchive& ar) override;
	int          MouseCreateCallback(IDisplayViewport* pView, EMouseEvent event, CPoint& point, int flags) override;
	virtual bool IsScalable() const override  { return !m_isEditMode; }
	virtual bool IsRotatable() const override { return !m_isEditMode; }

	void         EditSpline();

	// Serialization::ISpline
	virtual CryGUID GetOwnerInstanceGUID() const override { return GetId(); }

	virtual void StartEditing() override 
	{
		SetEditMode(true);
	}

	virtual void StopEditing() override
	{
		SetEditMode(false);
	}

	virtual Matrix34 GetWorldTransform() const override { return GetWorldTM(); }

	virtual void RemovePointByIndex(int index) override
	{
		if (index == GetSelectedPoint())
		{
			SelectPoint(-1);
		}
		RemovePoint(index);
	}

	virtual int InsertNewPoint(int index, const Vec3& position) override
	{
		int usedIndex = InsertPoint(index, position);
		SelectPoint(usedIndex);

		return usedIndex;
	}

	virtual void UpdatePointByIndex(int index, const Vec3& newPosition) override
	{
		SetPoint(index, newPosition);
	}

	virtual void OnMovePoint() override
	{
		CalcBBox();
		OnUpdate();
	}

	virtual void OnSelectedPointChanged() override
	{
		OnUpdateUI();
	}
	// ~Serialization::ISpline

protected:
	void         SerializeProperties(Serialization::IArchive& ar, bool bMultiEdit);

protected:
	AABB                       m_bbox;

	int                        m_mergeIndex;

	bool                       m_isEditMode;

	static class CSplinePanel* m_pSplinePanel;
	static int                 m_splineRollupID;
};

class CEditSplineObjectTool : public CEditTool, public ITransformManipulatorOwner
{
public:
	DECLARE_DYNCREATE(CEditSplineObjectTool)

	CEditSplineObjectTool() :
		m_pSpline(0),
		m_currPoint(-1),
		m_modifying(false),
		m_curCursor(STD_CURSOR_DEFAULT),
		m_pManipulator(nullptr)
	{}

	// Ovverides from CEditTool
	virtual string GetDisplayName() const override { return "Edit Spline"; }
	bool           MouseCallback(CViewport* view, EMouseEvent event, CPoint& point, int flags);
	void           OnManipulatorDrag(IDisplayViewport* pView, ITransformManipulator* pManipulator, const Vec2i& point0, const Vec3& value, int flags);
	void           OnManipulatorBegin(IDisplayViewport* view, ITransformManipulator* pManipulator, const Vec2i& point, int flags);
	void           OnManipulatorEnd(IDisplayViewport* view, ITransformManipulator* pManipulator);

	virtual void   SetUserData(const char* key, void* userData);

	virtual void   Display(DisplayContext& dc) {}
	virtual bool   OnKeyDown(CViewport* view, uint32 nChar, uint32 nRepCnt, uint32 nFlags);

	bool           IsNeedMoveTool() override { return true; }

	void           OnSplineEvent(CBaseObject* pObj, int evt);

	// ITransformManipulatorOwner
	virtual void GetManipulatorPosition(Vec3& position) override;
	virtual bool IsManipulatorVisible() override;

protected:
	virtual ~CEditSplineObjectTool();
	void DeleteThis() { delete this; }

	void SelectPoint(int index);
	void SetCursor(EStdCursor cursor, bool bForce = false);

	CBaseObject* GetSplineObject() const { return GetIEditor()->GetObjectManager()->FindObject(m_pSpline->GetOwnerInstanceGUID()); }

private:
	Serialization::ISpline* m_pSpline;
	int                     m_currPoint;
	bool                    m_modifying;
	CPoint                  m_mouseDownPos;
	Vec3                    m_pointPos;
	EStdCursor              m_curCursor;
	ITransformManipulator*  m_pManipulator;
};