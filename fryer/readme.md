# Fryer

Program that generates XML simulation files for the falafels project.

```
     Input:                                    Outputs:
                                         +------------------+
                                |------> |fried-falafels.xml|
+----------------+        +-----+        +------------------+
|raw-falafels.xml| -----> |Fryer|
+----------------+        +-----+       +--------------------+
                                |-----> |simgrid-platform.xml|
                                        +--------------------+
```

## XML parsing 

Under the hood, the fryer is implemented in Rust in order to utilize [Serde](https://serde.rs/)'s strength of serialization and 
deserialization.
Because XML is not supported by default by Serde, we use the library [quick-xml](https://docs.rs/quick-xml/latest/quick_xml/index.html)
to parse and write XML. 

## Serde usage with quick-xml

See: https://docs.rs/quick-xml/latest/quick_xml/de/index.html and
https://docs.rs/quick-xml/latest/quick_xml/se/index.html for more details.

In this project we are mostly using the following principles:

The usage of the macro "rename" is really important as XML Elements may contain other
Elements and/or multiple attributes. Thus we can access attributes with @ keyword (which
wouldn't be possible in the name of a struct field). 
```
#[serde(rename = "@id")] // Specify that the field bellow access the attribute "id"
pub id: String // the field cannot be named @id, that's why we rename it
```

In addition, we are also renaming fields that binds list of Elements.
```
#[serde(rename = "item")] // We have multiple Elements but they are individually called "item"
pub items: Vec<Item> // In rust we get a Vector of Item, so it makes more sense to call the
                      // field "items"
```

To access element content i.e. <elem>Content</elem> or the name of child element <elem><content></content><elem> we
rename our field to $value
```
#[serde(rename = "$value")]
pub content: Content
```

## Running

Note: prefer running in Debug mode because Rust doesn't check for integer overflow in release.
