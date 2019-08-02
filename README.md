# QUaAccessControl

Small test library implementing simplified access control information model, that can be integrated easily via *References* with other OPC UA objects using the [QUaServer](https://github.com/juangburgos/QUaServer) library.

This library only implements the information model, it is the responsaibility of the user to integrate it with its own information model, using [the mechanisms](https://github.com/juangburgos/QUaServer#users) provided by the [QUaServer](https://github.com/juangburgos/QUaServer) library.

Some simple use-cases are provided in [examples](./examples), where widgets are also provided to manage graphically the collections of objects used for access control.

## Information Model

The library uses 3 object types:

* User : Contains information about a user (name, password hash, etc.). A *User* object might be related to one *Role* object (using the *HasRole* reference). It might also be related to at least one *Permissions* object using the *CanRead* and/or *CanWrite* references). A *User* object must be related to at least one *Permissions* object or to a *Role* object to be of any use.

* Role : Groups multiple *User* objects that will share common permissions. A *Role* object must be related to at least one *Permissions* object to be of any use (using the *CanRead* and/or *CanWrite* references).

* Permissions : Groups multiple *User* and *Role* objects into two categories; *CanRead* and/or *CanWrite*. A *User* or *Role* object can belog to either, both or none of such categories. A *Permissions* object must be related to a user defined *External Object* to be of any use (using the *HasPermissions* reference).

For each of the object types above there exists a *Folder Object* type that works as factory for each of the types. They are called *UserList*, *RoleList* and *PermissionsList* respectively.

The *List* objects also implement serialization to XML to convinently backup and restore the information model state.

## Usage

TODO. 


