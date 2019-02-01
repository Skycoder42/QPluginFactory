# QPluginFactory
A factory class to easily load generic plugins

## Features
- Load plugins based on their IID and static keys
- Supports both dynamic and static plugins
- Respects the QT_PLUGIN_PATH and other plugin-related settings
- Supports factory-like generic plugin factory for such plugins

## Installation
The package is provided via qdep, as `Skycoder42/QPluginFactory`. To use it simply:

1. Install and enable qdep (See [qdep - Installing](https://github.com/Skycoder42/qdep#installation))
2. Add the following to your pro file:
```qmake
QDEP_DEPENDS += Skycoder42/QPluginFactory
!load(qdep):error("Failed to load qdep feature! Run 'qdep.py prfgen --qmake $$QMAKE_QMAKE' to create it.")
```

## Example
A full example with a demo plugin can be found in the `Sample` folder. The following code snippet illustrates how to use the factory:

### Interface
First, assume we have a plugin definition as follows:

```cpp
class MyClass;
class IMyPlugin
{
public:
	virtual inline ~IMyPlugin() = default;

	virtual MyClass *createInstance(const QString &key) = 0;
};

#define IMyPluginIID "de.skycoder42.qpluginloader.sample.IMyPlugin"
Q_DECLARE_INTERFACE(IMyPlugin, IMyPluginIID)
```

### Plugin
Next, create a plugin that implements this interface. The important parts here is to make shure that:

1. The plugin extends `QObect`
2. The plugin implements your interface with a JSON-Manifest
3. You declare the interface via `Q_INTERFACES`
4. The JSON-Manifest contains an element called `Keys` that holds an array of keys that identify the plugin

The plugin header could look like this:

```cpp
class MyPluginImplementation : public QObject, public IMyPlugin
{
	Q_OBJECT
	Q_INTERFACES(IMyPlugin)
	Q_PLUGIN_METADATA(IID IMyPluginIID FILE "MyPluginImplementation.json")

public:
	//...
};
```

And the corresponding JSON-Manifest as follows. For the example, we want the plugin to be identified by the key `"stuff"`, but can can of course specify anything here, whatever fits your semantics best. Multiple keys are allowed as well.

```json
{
	"Keys" : [ "stuff" ]
}
```

### Application
We can use the factory to create instances of this plugin by simply creating a generic instance. There are prepared macros that do so by creating a `Q_GLOBAL_STATIC` instance of the factory. But you can of course also create the instance yourself. The following code creates a `QPluginFactory<IMyPlugin>` instanced named `loader` that searches for plugins in a subfolder to the default Qt plugin folder (or a folder advertised via `QT_PLUGIN_PATH`) named `myPlugin`:
```cpp
Q_GLOBAL_PLUGIN_FACTORY(IMyPlugin, "myPlugin", loader)

// ...

IMyPlugin *instance = loader->plugin("stuff");
```

If you want to use the factory style instead, use the following:
```cpp
Q_GLOBAL_PLUGIN_OBJECT_FACTORY(IMyPlugin, MyClass, "myPlugin", loader)

// ...

MyClass *instance = loader->createInstance("stuff");
```
