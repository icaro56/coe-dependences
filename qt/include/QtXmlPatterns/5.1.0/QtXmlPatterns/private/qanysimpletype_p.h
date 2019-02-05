/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtXmlPatterns module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef Patternist_AnySimpleType_H
#define Patternist_AnySimpleType_H

#include <private/qanytype_p.h>

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    class AtomicType;

    /**
     * @short Represents the @c xs:anySimpleType item type.
     *
     * @ingroup Patternist_types
     * @see <a href="http://www.w3.org/TR/xmlschema-2/#dt-anySimpleType">XML Schema Part 2:
     * Datatypes Second Edition, The simple ur-type definition</a>
     * @author Frans Englich <frans.englich@nokia.com>
     */
    class AnySimpleType : public AnyType
    {
    public:
        typedef QExplicitlySharedDataPointer<AnySimpleType> Ptr;
        typedef QList<AnySimpleType::Ptr> List;
        friend class BuiltinTypes;

        virtual ~AnySimpleType();

        virtual QXmlName name(const NamePool::Ptr &np) const;

        /**
         * @returns always @c xs:anySimpleType
         */
        virtual QString displayName(const NamePool::Ptr &np) const;

        /**
         * @returns always BuiltinTypes::xsAnyType
         */
        virtual SchemaType::Ptr wxsSuperType() const;

        /**
         * xs:anySimpleType is the special "simple ur-type". Therefore this function
         * returns SchemaType::None
         *
         * @returns SchemaType::None
         */
        virtual TypeCategory category() const;

        /**
         * The simple ur-type is a "special restriction of the ur-type definition",
         * according to XML Schema Part 2: Datatypes Second Edition about xs:anySimpleType
         *
         * @returns DERIVATION_RESTRICTION
         */
        virtual SchemaType::DerivationMethod derivationMethod() const;

        /**
         * Always returns @c true.
         */
        virtual bool isSimpleType() const;

        /**
         * Always returns @c false.
         */
        virtual bool isComplexType() const;

    protected:
        AnySimpleType();

    };
}

QT_END_NAMESPACE

#endif
