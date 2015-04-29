/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "config.h"

#if ENABLE(SVG)

#include "JSSVGUseElement.h"

#include "CSSMutableStyleDeclaration.h"
#include "CSSStyleDeclaration.h"
#include "CSSValue.h"
#include "JSCSSStyleDeclaration.h"
#include "JSCSSValue.h"
#include "JSSVGAnimatedBoolean.h"
#include "JSSVGAnimatedLength.h"
#include "JSSVGAnimatedString.h"
#include "JSSVGAnimatedTransformList.h"
#include "JSSVGElement.h"
#include "JSSVGElementInstance.h"
#include "JSSVGMatrix.h"
#include "JSSVGRect.h"
#include "JSSVGStringList.h"
#include "KURL.h"
#include "SVGElement.h"
#include "SVGElementInstance.h"
#include "SVGStringList.h"
#include "SVGUseElement.h"
#include <runtime/Error.h>
#include <runtime/JSString.h>
#include <wtf/GetPtr.h>

using namespace JSC;

namespace WebCore {

ASSERT_CLASS_FITS_IN_CELL(JSSVGUseElement);

/* Hash table */

static const HashTableValue JSSVGUseElementTableValues[20] =
{
    { "x", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementX), (intptr_t)0 },
    { "y", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementY), (intptr_t)0 },
    { "width", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementWidth), (intptr_t)0 },
    { "height", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementHeight), (intptr_t)0 },
    { "instanceRoot", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementInstanceRoot), (intptr_t)0 },
    { "animatedInstanceRoot", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementAnimatedInstanceRoot), (intptr_t)0 },
    { "href", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementHref), (intptr_t)0 },
    { "requiredFeatures", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementRequiredFeatures), (intptr_t)0 },
    { "requiredExtensions", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementRequiredExtensions), (intptr_t)0 },
    { "systemLanguage", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementSystemLanguage), (intptr_t)0 },
    { "xmllang", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementXmllang), (intptr_t)setJSSVGUseElementXmllang },
    { "xmlspace", DontDelete, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementXmlspace), (intptr_t)setJSSVGUseElementXmlspace },
    { "externalResourcesRequired", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementExternalResourcesRequired), (intptr_t)0 },
    { "className", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementClassName), (intptr_t)0 },
    { "style", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementStyle), (intptr_t)0 },
    { "transform", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementTransform), (intptr_t)0 },
    { "nearestViewportElement", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementNearestViewportElement), (intptr_t)0 },
    { "farthestViewportElement", DontDelete|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementFarthestViewportElement), (intptr_t)0 },
    { "constructor", DontEnum|ReadOnly, (intptr_t)static_cast<PropertySlot::GetValueFunc>(jsSVGUseElementConstructor), (intptr_t)0 },
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSSVGUseElementTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 1023, JSSVGUseElementTableValues, 0 };
#else
    { 68, 63, JSSVGUseElementTableValues, 0 };
#endif

/* Hash table for constructor */

static const HashTableValue JSSVGUseElementConstructorTableValues[1] =
{
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSSVGUseElementConstructorTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 0, JSSVGUseElementConstructorTableValues, 0 };
#else
    { 1, 0, JSSVGUseElementConstructorTableValues, 0 };
#endif

class JSSVGUseElementConstructor : public DOMConstructorObject {
public:
    JSSVGUseElementConstructor(ExecState* exec, JSDOMGlobalObject* globalObject)
        : DOMConstructorObject(JSSVGUseElementConstructor::createStructure(globalObject->objectPrototype()), globalObject)
    {
        putDirect(exec->propertyNames().prototype, JSSVGUseElementPrototype::self(exec, globalObject), None);
    }
    virtual bool getOwnPropertySlot(ExecState*, const Identifier&, PropertySlot&);
    virtual bool getOwnPropertyDescriptor(ExecState*, const Identifier&, PropertyDescriptor&);
    virtual const ClassInfo* classInfo() const { return &s_info; }
    static const ClassInfo s_info;

    static PassRefPtr<Structure> createStructure(JSValue proto) 
    { 
        return Structure::create(proto, TypeInfo(ObjectType, StructureFlags), AnonymousSlotCount); 
    }
    
protected:
    static const unsigned StructureFlags = OverridesGetOwnPropertySlot | ImplementsHasInstance | DOMConstructorObject::StructureFlags;
};

const ClassInfo JSSVGUseElementConstructor::s_info = { "SVGUseElementConstructor", 0, &JSSVGUseElementConstructorTable, 0 };

bool JSSVGUseElementConstructor::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSSVGUseElementConstructor, DOMObject>(exec, &JSSVGUseElementConstructorTable, this, propertyName, slot);
}

bool JSSVGUseElementConstructor::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSSVGUseElementConstructor, DOMObject>(exec, &JSSVGUseElementConstructorTable, this, propertyName, descriptor);
}

/* Hash table for prototype */

static const HashTableValue JSSVGUseElementPrototypeTableValues[7] =
{
    { "hasExtension", DontDelete|Function, (intptr_t)static_cast<NativeFunction>(jsSVGUseElementPrototypeFunctionHasExtension), (intptr_t)1 },
    { "getPresentationAttribute", DontDelete|Function, (intptr_t)static_cast<NativeFunction>(jsSVGUseElementPrototypeFunctionGetPresentationAttribute), (intptr_t)1 },
    { "getBBox", DontDelete|Function, (intptr_t)static_cast<NativeFunction>(jsSVGUseElementPrototypeFunctionGetBBox), (intptr_t)0 },
    { "getCTM", DontDelete|Function, (intptr_t)static_cast<NativeFunction>(jsSVGUseElementPrototypeFunctionGetCTM), (intptr_t)0 },
    { "getScreenCTM", DontDelete|Function, (intptr_t)static_cast<NativeFunction>(jsSVGUseElementPrototypeFunctionGetScreenCTM), (intptr_t)0 },
    { "getTransformToElement", DontDelete|Function, (intptr_t)static_cast<NativeFunction>(jsSVGUseElementPrototypeFunctionGetTransformToElement), (intptr_t)1 },
    { 0, 0, 0, 0 }
};

static JSC_CONST_HASHTABLE HashTable JSSVGUseElementPrototypeTable =
#if ENABLE(PERFECT_HASH_SIZE)
    { 2047, JSSVGUseElementPrototypeTableValues, 0 };
#else
    { 17, 15, JSSVGUseElementPrototypeTableValues, 0 };
#endif

const ClassInfo JSSVGUseElementPrototype::s_info = { "SVGUseElementPrototype", 0, &JSSVGUseElementPrototypeTable, 0 };

JSObject* JSSVGUseElementPrototype::self(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMPrototype<JSSVGUseElement>(exec, globalObject);
}

bool JSSVGUseElementPrototype::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticFunctionSlot<JSObject>(exec, &JSSVGUseElementPrototypeTable, this, propertyName, slot);
}

bool JSSVGUseElementPrototype::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticFunctionDescriptor<JSObject>(exec, &JSSVGUseElementPrototypeTable, this, propertyName, descriptor);
}

const ClassInfo JSSVGUseElement::s_info = { "SVGUseElement", &JSSVGElement::s_info, &JSSVGUseElementTable, 0 };

JSSVGUseElement::JSSVGUseElement(NonNullPassRefPtr<Structure> structure, JSDOMGlobalObject* globalObject, PassRefPtr<SVGUseElement> impl)
    : JSSVGElement(structure, globalObject, impl)
{
}

JSObject* JSSVGUseElement::createPrototype(ExecState* exec, JSGlobalObject* globalObject)
{
    return new (exec) JSSVGUseElementPrototype(JSSVGUseElementPrototype::createStructure(JSSVGElementPrototype::self(exec, globalObject)));
}

bool JSSVGUseElement::getOwnPropertySlot(ExecState* exec, const Identifier& propertyName, PropertySlot& slot)
{
    return getStaticValueSlot<JSSVGUseElement, Base>(exec, &JSSVGUseElementTable, this, propertyName, slot);
}

bool JSSVGUseElement::getOwnPropertyDescriptor(ExecState* exec, const Identifier& propertyName, PropertyDescriptor& descriptor)
{
    return getStaticValueDescriptor<JSSVGUseElement, Base>(exec, &JSSVGUseElementTable, this, propertyName, descriptor);
}

JSValue jsSVGUseElementX(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    RefPtr<SVGAnimatedLength> obj = imp->xAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGUseElementY(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    RefPtr<SVGAnimatedLength> obj = imp->yAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGUseElementWidth(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    RefPtr<SVGAnimatedLength> obj = imp->widthAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGUseElementHeight(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    RefPtr<SVGAnimatedLength> obj = imp->heightAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGUseElementInstanceRoot(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->instanceRoot()));
    return result;
}

JSValue jsSVGUseElementAnimatedInstanceRoot(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->animatedInstanceRoot()));
    return result;
}

JSValue jsSVGUseElementHref(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    RefPtr<SVGAnimatedString> obj = imp->hrefAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGUseElementRequiredFeatures(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->requiredFeatures()), imp);
    return result;
}

JSValue jsSVGUseElementRequiredExtensions(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->requiredExtensions()), imp);
    return result;
}

JSValue jsSVGUseElementSystemLanguage(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->systemLanguage()), imp);
    return result;
}

JSValue jsSVGUseElementXmllang(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->xmllang());
    return result;
}

JSValue jsSVGUseElementXmlspace(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    JSValue result = jsString(exec, imp->xmlspace());
    return result;
}

JSValue jsSVGUseElementExternalResourcesRequired(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    RefPtr<SVGAnimatedBoolean> obj = imp->externalResourcesRequiredAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGUseElementClassName(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    RefPtr<SVGAnimatedString> obj = imp->classNameAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGUseElementStyle(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->style()));
    return result;
}

JSValue jsSVGUseElementTransform(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    RefPtr<SVGAnimatedTransformList> obj = imp->transformAnimated();
    JSValue result =  toJS(exec, castedThis->globalObject(), obj.get(), imp);
    return result;
}

JSValue jsSVGUseElementNearestViewportElement(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->nearestViewportElement()));
    return result;
}

JSValue jsSVGUseElementFarthestViewportElement(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* castedThis = static_cast<JSSVGUseElement*>(asObject(slotBase));
    UNUSED_PARAM(exec);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThis->impl());
    JSValue result = toJS(exec, castedThis->globalObject(), WTF::getPtr(imp->farthestViewportElement()));
    return result;
}

JSValue jsSVGUseElementConstructor(ExecState* exec, JSValue slotBase, const Identifier&)
{
    JSSVGUseElement* domObject = static_cast<JSSVGUseElement*>(asObject(slotBase));
    return JSSVGUseElement::getConstructor(exec, domObject->globalObject());
}
void JSSVGUseElement::put(ExecState* exec, const Identifier& propertyName, JSValue value, PutPropertySlot& slot)
{
    lookupPut<JSSVGUseElement, Base>(exec, propertyName, value, &JSSVGUseElementTable, this, slot);
}

void setJSSVGUseElementXmllang(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSSVGUseElement* castedThisObj = static_cast<JSSVGUseElement*>(thisObject);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThisObj->impl());
    imp->setXmllang(value.toString(exec));
}

void setJSSVGUseElementXmlspace(ExecState* exec, JSObject* thisObject, JSValue value)
{
    JSSVGUseElement* castedThisObj = static_cast<JSSVGUseElement*>(thisObject);
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThisObj->impl());
    imp->setXmlspace(value.toString(exec));
}

JSValue JSSVGUseElement::getConstructor(ExecState* exec, JSGlobalObject* globalObject)
{
    return getDOMConstructor<JSSVGUseElementConstructor>(exec, static_cast<JSDOMGlobalObject*>(globalObject));
}

JSValue JSC_HOST_CALL jsSVGUseElementPrototypeFunctionHasExtension(ExecState* exec, JSObject*, JSValue thisValue, const ArgList& args)
{
    UNUSED_PARAM(args);
    if (!thisValue.inherits(&JSSVGUseElement::s_info))
        return throwError(exec, TypeError);
    JSSVGUseElement* castedThisObj = static_cast<JSSVGUseElement*>(asObject(thisValue));
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThisObj->impl());
    const UString& extension = args.at(0).toString(exec);


    JSC::JSValue result = jsBoolean(imp->hasExtension(extension));
    return result;
}

JSValue JSC_HOST_CALL jsSVGUseElementPrototypeFunctionGetPresentationAttribute(ExecState* exec, JSObject*, JSValue thisValue, const ArgList& args)
{
    UNUSED_PARAM(args);
    if (!thisValue.inherits(&JSSVGUseElement::s_info))
        return throwError(exec, TypeError);
    JSSVGUseElement* castedThisObj = static_cast<JSSVGUseElement*>(asObject(thisValue));
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThisObj->impl());
    const UString& name = args.at(0).toString(exec);


    JSC::JSValue result = toJS(exec, castedThisObj->globalObject(), WTF::getPtr(imp->getPresentationAttribute(name)));
    return result;
}

JSValue JSC_HOST_CALL jsSVGUseElementPrototypeFunctionGetBBox(ExecState* exec, JSObject*, JSValue thisValue, const ArgList& args)
{
    UNUSED_PARAM(args);
    if (!thisValue.inherits(&JSSVGUseElement::s_info))
        return throwError(exec, TypeError);
    JSSVGUseElement* castedThisObj = static_cast<JSSVGUseElement*>(asObject(thisValue));
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThisObj->impl());


    JSC::JSValue result = toJS(exec, castedThisObj->globalObject(), JSSVGStaticPODTypeWrapper<FloatRect>::create(imp->getBBox()).get(), 0 /* no context on purpose */);
    return result;
}

JSValue JSC_HOST_CALL jsSVGUseElementPrototypeFunctionGetCTM(ExecState* exec, JSObject*, JSValue thisValue, const ArgList& args)
{
    UNUSED_PARAM(args);
    if (!thisValue.inherits(&JSSVGUseElement::s_info))
        return throwError(exec, TypeError);
    JSSVGUseElement* castedThisObj = static_cast<JSSVGUseElement*>(asObject(thisValue));
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThisObj->impl());


    JSC::JSValue result = toJS(exec, castedThisObj->globalObject(), JSSVGStaticPODTypeWrapper<AffineTransform>::create(imp->getCTM()).get(), 0 /* no context on purpose */);
    return result;
}

JSValue JSC_HOST_CALL jsSVGUseElementPrototypeFunctionGetScreenCTM(ExecState* exec, JSObject*, JSValue thisValue, const ArgList& args)
{
    UNUSED_PARAM(args);
    if (!thisValue.inherits(&JSSVGUseElement::s_info))
        return throwError(exec, TypeError);
    JSSVGUseElement* castedThisObj = static_cast<JSSVGUseElement*>(asObject(thisValue));
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThisObj->impl());


    JSC::JSValue result = toJS(exec, castedThisObj->globalObject(), JSSVGStaticPODTypeWrapper<AffineTransform>::create(imp->getScreenCTM()).get(), 0 /* no context on purpose */);
    return result;
}

JSValue JSC_HOST_CALL jsSVGUseElementPrototypeFunctionGetTransformToElement(ExecState* exec, JSObject*, JSValue thisValue, const ArgList& args)
{
    UNUSED_PARAM(args);
    if (!thisValue.inherits(&JSSVGUseElement::s_info))
        return throwError(exec, TypeError);
    JSSVGUseElement* castedThisObj = static_cast<JSSVGUseElement*>(asObject(thisValue));
    SVGUseElement* imp = static_cast<SVGUseElement*>(castedThisObj->impl());
    ExceptionCode ec = 0;
    SVGElement* element = toSVGElement(args.at(0));


    JSC::JSValue result = toJS(exec, castedThisObj->globalObject(), JSSVGStaticPODTypeWrapper<AffineTransform>::create(imp->getTransformToElement(element, ec)).get(), 0 /* no context on purpose */);
    setDOMException(exec, ec);
    return result;
}


}

#endif // ENABLE(SVG)
