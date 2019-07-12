* Protect XML file for corruptions with secret hash.

* Create optional all powerful root user? (create it in user list, define it as an optional ref or instance in ac, overwrite the userCanRead/Write in permissions object)

* Auto create permissions for any new user "user only permissions" (findout how to name it) and assign it to the new user. Is user gets deleted, also delete this permissions.

* Fro every new permissions instance create, assign to it the "user only permissions" of the user that created the permissions instance.
