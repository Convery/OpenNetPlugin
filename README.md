# OpenNet - Plugin build

This project aims to provide a base for network emulation with a module system for extension.
The dll version emulates connections locally while the server version allows modules to be used anywhere.
The plugin version wraps the dll version to be used with some external emulation projects. Currently for Windows, cross platform support is planned.

# Modules
Modules are dlls with a single export, CreateServer. 
This export returns a IServer pointer to a class in the module. 
Thus the module needs to fully implement IServer, examples will be posted in the future.

Modules allow developers to create both closed and opensource modules.
Opennet has support for decrypting modules based on license strings.
Developers can thus license their modules to projects and customers, as well as knowing if a module has been stolen/leaked/pirated.

# Legal
As with all general tools, this can be used for a myriad of tools and projects.
Our goal is mainly to help game developers, anti-malware researchers and academics to work offline.
Contributors to this project are not to be held liable for whatever users do.
So stop sending me C&D letters.
