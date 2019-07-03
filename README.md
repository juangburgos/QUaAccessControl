# QUaAccessControl

Small library implementing simplified access control information model, that can be integrated easily via *References* with other OPC UA objects using the [QUaServer](https://github.com/juangburgos/QUaServer) library.

This library only implements the information model, it is the responsaibility of the user to integrate it with its own information model, using [the mechanisms](https://github.com/juangburgos/QUaServer#users) provided by the [QUaServer](https://github.com/juangburgos/QUaServer) library.

Some simple use-cases are provided in [examples](./examples).

## Information Model

The library uses 3 object types:

* User : Contains information about a user (name, password, etc.). A *User* object must have one *Role* object assigned to be of any use (using the *HasRole* reference).

* Role : Groups multiple *User* objects that will share common permissions. A *Role* object must be related to at least one *Permissions* object to be of any use (Using the *CanRead* and/or *CanWrite* references).

* Permissions : Groups multiple *Role* objects into two categories; *CanRead* and *CanWrite*. A *Role* object can belog to either, both or none of such categories. A *Permissions* object must be related to a user defined *External Object* to be of any use (Using the *HasPermissions* reference).

For each of the object types above there exists a *Folder Object* type that works as factory for each of the types. They are called *UserList*, *RoleList* and *PermissionsList* respectively.

The *List* objects also implement serialization to XML to convinently backup and restore the information model state.

## Usage

TODO. 

QUaUserList, QUaUser, QUaRoleList, QUaRole, QUaPermissionsList, QUaPermissions

## TODO

Need to modify, add a callback or something to QUaServer to allow external validation. E.g. transforming plain passwords into hashes and validate the hashes.