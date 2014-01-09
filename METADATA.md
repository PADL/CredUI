Provider metadata
=================

Providers can configure the following properties in Info.plist.

CUISupportedClasses
-------------------

An array of strings indicating the kCUIAttrClass accepted by this provider. Optional; if absent, the provider must do any filtering. Class values are either kCUIAttrClass constants or GSS OIDs.

CUIGenericPersistenceProviderFactory
------------------------------------

Factory UUID of the provider used for serialising generic credentials. Used by password provider only.

CUIPersistenceProviderFactory
-----------------------------

Factory UUID of the provider used for serialising non-generic credentials. Used by password provider only.

CUIIsPersistenceProvider
------------------------

True if the provider is a persistence provider.