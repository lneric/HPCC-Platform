/*##############################################################################

    Copyright (C) 2011 HPCC Systems.

    All rights reserved. This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
############################################################################## */

#include "hqlrepository.hpp"
#include "hqlerrors.hpp"
#include "hqlplugins.hpp"
#include "jdebug.hpp"
#include "jfile.hpp"
#include "eclrtl.hpp"
#include "hqlexpr.ipp"
#include "hqlerror.hpp"

//-------------------------------------------------------------------------------------------------------------------

static void getRootScopes(HqlScopeArray & rootScopes, IHqlScope * scope)
{
    HqlExprArray rootSymbols;
    scope->getSymbols(rootSymbols);
    ForEachItemIn(i, rootSymbols)
    {
        IHqlExpression & cur = rootSymbols.item(i);
        IHqlScope * scope = cur.queryScope();
        if (scope)
            rootScopes.append(*LINK(scope));
    }
}

void getRootScopes(HqlScopeArray & rootScopes, IHqlScope * scope, HqlLookupContext & ctx)
{
    scope->ensureSymbolsDefined(ctx);
    getRootScopes(rootScopes, scope);
}

void getRootScopes(HqlScopeArray & rootScopes, IEclRepository * repository, HqlLookupContext & ctx)
{
    getRootScopes(rootScopes, repository->queryRootScope(), ctx);
}

void getImplicitScopes(HqlScopeArray& implicitScopes, IEclRepository * repository, IHqlScope * scope, HqlLookupContext & ctx)
{
    //Any implicit scope requires explicit module imports
    if (scope->isImplicit())
        return;

    //See note before CMergedScope for notes about implicit scopes.
    HqlScopeArray rootScopes;
    getRootScopes(rootScopes, repository->queryRootScope(), ctx);
    ForEachItemIn(i, rootScopes)
    {
        IHqlScope & scope = rootScopes.item(i);
        if (scope.isImplicit())
            implicitScopes.append(OLINK(scope));
    }
}


extern HQL_API void importRootModulesToScope(IHqlScope * scope, HqlLookupContext & ctx)
{
    IEclRepository * eclRepository = ctx.queryRepository();
    HqlScopeArray rootScopes;
    getRootScopes(rootScopes, eclRepository, ctx);
    ForEachItemIn(i, rootScopes)
    {
        IHqlScope & cur = rootScopes.item(i);
        _ATOM curName = cur.queryName();
        OwnedHqlExpr resolved = eclRepository->queryRootScope()->lookupSymbol(curName, LSFpublic, ctx);
        if (resolved)
            scope->defineSymbol(curName, NULL, resolved.getClear(), false, true, ob_import);
    }
}

//-------------------------------------------------------------------------------------------------------------------

void lookupAllRootDefinitions(IHqlScope * scope, HqlLookupContext & ctx)
{
    HqlExprArray rootSymbols;
    scope->getSymbols(rootSymbols);
    ForEachItemIn(i, rootSymbols)
    {
        ::Release(scope->lookupSymbol(rootSymbols.item(i).queryName(), LSFsharedOK, ctx));
    }
}

void lookupAllRootDefinitions(IEclRepository * repository)
{
    HqlParseContext parseCtx(repository, NULL);
    ThrowingErrorReceiver errs;
    HqlLookupContext ctx(parseCtx, &errs);
    lookupAllRootDefinitions(repository->queryRootScope(), ctx);
}

//-------------------------------------------------------------------------------------------------------------------

IHqlExpression * getResolveAttributeFullPath(const char * attrname, unsigned lookupFlags, HqlLookupContext & ctx)
{
    Owned<IHqlScope> parentScope;
    const char * item = attrname;
    const char * dot;
    do
    {
        dot = strchr(item, '.');
        _ATOM moduleName;
        if (dot)
        {
            moduleName = createIdentifierAtom(item, dot-item);
            item = dot + 1;
        }
        else
        {
            moduleName = createIdentifierAtom(item);
        }

        OwnedHqlExpr resolved;
        if (parentScope)
        {
            resolved.setown(parentScope->lookupSymbol(moduleName, lookupFlags, ctx));
        }
        else
        {
            resolved.setown(ctx.queryRepository()->queryRootScope()->lookupSymbol(moduleName, lookupFlags, ctx));
        }

        if (!resolved || !dot)
            return resolved.getClear();
        IHqlScope * scope = resolved->queryScope();

        if (!scope)
            return NULL;

        parentScope.set(scope);
    } while (dot);
    return LINK(queryExpression(parentScope));
}


IHqlScope * getResolveDottedScope(const char * modname, unsigned lookupFlags, HqlLookupContext & ctx)
{
    if (!modname || !*modname)
        return LINK(ctx.queryRepository()->queryRootScope());
    OwnedHqlExpr matched = getResolveAttributeFullPath(modname, lookupFlags, ctx);
    if (matched)
        return LINK(matched->queryScope());
    return NULL;
}


//-------------------------------------------------------------------------------------------------------------------

class HQL_API CompoundEclRepository : public CInterface, implements IEclRepository
{
public:
    CompoundEclRepository() { rootScope.setown(new CHqlMergedScope(NULL, NULL)); }

    IMPLEMENT_IINTERFACE;

    void addRepository(IEclRepository & _repository);

    virtual IHqlScope * queryRootScope() { return rootScope; }
    virtual void checkCacheValid();

protected:
    IArrayOf<IEclRepository> repositories;
    Owned<CHqlMergedScope> rootScope;
};

void CompoundEclRepository::addRepository(IEclRepository & _repository)
{
    repositories.append(OLINK(_repository));
    rootScope->addScope(_repository.queryRootScope());
}

//-------------------------------------------------------------------------------------------------------------------

void CompoundEclRepository::checkCacheValid()
{
    ForEachItemIn(i, repositories)
        repositories.item(i).checkCacheValid();
}


extern HQL_API IEclRepository * createCompoundRepositoryF(IEclRepository * repository, ...)
{
    Owned<CompoundEclRepository> compound = new CompoundEclRepository;
    compound->addRepository(*repository);
    va_list args;
    va_start(args, repository);
    for (;;)
    {
        IEclRepository * next = va_arg(args, IEclRepository*);
        if (!next)
            break;
        compound->addRepository(*next);
    }
    return compound.getClear();
}


extern HQL_API IEclRepository * createCompoundRepository(EclRepositoryArray & repositories)
{
    Owned<CompoundEclRepository> compound = new CompoundEclRepository;
    ForEachItemIn(i, repositories)
        compound->addRepository(repositories.item(i));
    return compound.getClear();
}


//-------------------------------------------------------------------------------------------------------------------

class HQL_API CNewEclRepository : public CInterface, implements IEclRepositoryCallback
{
public:
    CNewEclRepository(IEclSourceCollection * _collection) : collection(_collection)
    {
        rootScope.setown(createRemoteScope(NULL, NULL, this, NULL, NULL, true, NULL));
    }
    IMPLEMENT_IINTERFACE

    virtual IHqlScope * queryRootScope() { return rootScope->queryScope(); }
    virtual void checkCacheValid() { collection->checkCacheValid(); }
    virtual bool loadModule(IHqlRemoteScope *scope, IErrorReceiver *errs, bool forceAll);
    virtual IHqlExpression * loadSymbol(IHqlRemoteScope *scope, IAtom * searchName);

protected:
    IHqlExpression * createSymbol(IHqlRemoteScope * rScope, IEclSource * source);

protected:
    Linked<IEclSourceCollection> collection;
    Owned<IHqlRemoteScope> rootScope;
    CriticalSection cs;
};



bool CNewEclRepository::loadModule(IHqlRemoteScope * rScope, IErrorReceiver *errs, bool forceAll)
{
    IEclSource * parent = rScope->queryEclSource();
    CHqlRemoteScope * targetScope = static_cast<CHqlRemoteScope *>(rScope);
    Owned<IEclSourceIterator> iter = collection->getContained(parent);
    if (iter)
    {
        ForEach(*iter)
        {
            IEclSource * next = &iter->query();
            Owned<IHqlExpression> element = createSymbol(rScope, next);
            targetScope->defineSymbol(LINK(element));
        }
    }
    return true;
}

IHqlExpression * CNewEclRepository::loadSymbol(IHqlRemoteScope * rScope, IAtom * searchName)
{
    IEclSource * parent = rScope->queryEclSource();
    Owned<IEclSource> source = collection->getSource(parent, searchName);
    return createSymbol(rScope, source);
}


static void getFullName(StringBuffer & fullName, IHqlScope * scope, _ATOM attrName)
{
    fullName.append(scope->queryFullName());
    if (fullName.length())
        fullName.append(".");
    fullName.append(attrName);
}


IHqlExpression * CNewEclRepository::createSymbol(IHqlRemoteScope * rScope, IEclSource * source)
{
    _ATOM eclName = source->queryEclName();
    IHqlScope * scope = rScope->queryScope();
    StringBuffer fullName;
    getFullName(fullName, scope, eclName);

    EclSourceType sourceType = source->queryType();
    IFileContents * contents = source->queryFileContents();
    OwnedHqlExpr body;
    unsigned symbolFlags = 0;
    switch (sourceType)
    {
    case ESTdefinition:
        {
            Owned<IProperties> props = source->getProperties();
            if (props)
            {
                unsigned flags = props->getPropInt(flagsAtom->str(), 0);
                if (flags & ob_sandbox)
                    symbolFlags |= ob_sandbox;
            }

            /*
            int flags = t->getPropInt("@flags", 0);
            if(access >= cs_read)
                flags|=ob_showtext;
                */
            break;
        }
    case ESTplugin:
    case ESTmodule:
    case ESTlibrary:
        {
            //Slightly ugly create a "delayed" nested scope instead.  But with a NULL owner - so will never be called back
            //Probably should be a difference class instance
            IProperties * props = source->getProperties();
            Owned<IHqlRemoteScope> childScope = createRemoteScope(eclName, fullName.str(), NULL, props, contents, true, source);
            body.set(queryExpression(childScope->queryScope()));
            break;
        }
    case ESTcontainer:
        {
            IProperties * props = source->getProperties();
            Owned<IHqlRemoteScope> childScope = createRemoteScope(eclName, fullName.str(), this, props, NULL, true, source);
            body.set(queryExpression(childScope->queryScope()));
            break;
        }
    default:
        throwUnexpected();
    }
    return CHqlNamedSymbol::makeSymbol(eclName, scope->queryName(), body.getClear(), NULL, true, true, symbolFlags, contents, 0);
}


extern HQL_API IEclRepository * createRepository(IEclSourceCollection * source)
{
    return new CNewEclRepository(source);
}

extern HQL_API IEclRepository * createRepository(EclSourceCollectionArray & sources)
{
    if (sources.ordinality() == 0)
        return NULL;
    if (sources.ordinality() == 1)
        return createRepository(&sources.item(0));

    EclRepositoryArray repositories;
    ForEachItemIn(i, sources)
        repositories.append(*createRepository(&sources.item(i)));
    return createCompoundRepository(repositories);
}

//-------------------------------------------------------------------------------------------------------------------

#include "hqlcollect.hpp"

extern HQL_API IEclRepository * createNewSourceFileEclRepository(IErrorReceiver *errs, const char * path, unsigned flags, unsigned trace)
{
    Owned<IEclSourceCollection> source = createFileSystemEclCollection(errs, path, flags, trace);
    return createRepository(source);
}

extern HQL_API IEclRepository * createSingleDefinitionEclRepository(const char * moduleName, const char * attrName, const char * text)
{
    Owned<IEclSourceCollection> source = createSingleDefinitionEclCollection(moduleName, attrName, text);
    return createRepository(source);
}
