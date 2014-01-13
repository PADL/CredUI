[![Build Status](https://travis-ci.org/PADL/CredUI.png?branch=master)](https://travis-ci.org/PADL/CredUI)

CredUI
======

Introduction
------------

CredUI is an OS X framework for interactively acquiring credentials. It is loosely modelled on the equivalent APIs introduced in Windows Vista, although these APIs are somewhat cleaner.

The purpose of CredUI is to acquire information from a user which an application can then use to acquire a credential for authenticating, typically using the GSS-API. *Credentials* are essentially a dictionary of attributes. *Credential providers* are responsible for enumerating credentials and describing the user interface with which credentials can be acquired (although the providers do not render the interface).

The motivation is part of an attempt to make OS X security mechanism agnostic. GSS-API mechanisms cannot safely display UI themselves and, if they do, the user risks being prompted multiple times by different mechanisms, or not being prompted until it's too late to choose an alternative mechanism. We need CredUI for new security mechanisms such as GSS EAP and BrowserID, that use alternative credential types (e.g. assertions) or require additional user interaction for confirming trust anchors, amongst other things.

There are three components: CredUICore, CredUI and the credential providers themselves.

CredUICore
----------

CredUICore is a CoreFoundation library; it does not require Foundation, AppKit or libobjc, and it's suitable for console applications to use (for example, a PAM module might use this). It acts as a broker between the user interface and the credential providers.

CredUI
------

A Cocoa wrapper for CredUICore that displays a (presently not particularly great looking) interface. The CUIIdentityPicker class is the principal way to use this API, although functions are also supplied. The identity picker can run as a modal panel or a sheet. Once finished, it provides a CUICredential object which can be used directly or to acquire a GSS-API credential handle.

Providers
---------

The Password credential provider acquires a username and password from the user.

The Certificate credential provider enumerates the keychain for private keys.

The GSSItem and Keychain providers are ordinary providers, but they call back into CredUICore to wrap other providers with any enumerated persisted credentials. This decouples the persistence mechanism from credential acquisition interface. One could write, say, an SQLite persistence provider and it would simply act as an attribute store.

The Login provider is another special type of provider, an identity provider. This behaves similarly to a persistence provider except it only stores usernames and associated metadata (such as an image), not the actual that can be used for authenticating. Identity providers are used for the kCUIUsageScenarioLogin case only and allow a user to select from an existing username.

GSS-API
-------

CredUI works best with GSSKit, owing to various limitations in the current Heimdal implementation on OS X. GSSKitUI glues the two frameworks together and provides a simple interface for acquiring GSS-API contexts, prompting the user using CredUI as necessary.


