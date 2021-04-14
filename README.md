# sjp

*sjp* is a Java parser for the Soufflé datalog engine. Its main
purpose is to enable program transformations and metaprogramming
of Java source code.

The parser is a work in progress but aims to be compliant with
the Java SE 15 grammar specification.

## Usage

In progress

## Performance

The parser performs well for practical use cases. It parses files
with < 1000 lines in ~10 ms on a consumer grade computer. Larger
files, say ~10000 lines, may take closer to ~500 ms.

The parser usually exhibits linear runtime behavior in practice,
however, it is possible to construct inputs where the parser
suffers from quadratic runtime behavior. This occurs when the
parser has to construct long sequential lists of one AST node
type, such as when a class has a lots of methods in a sequence or
a compilation unit has lots of classes in a sequence. If you need
to parse such files you might run into issues.

## Example

The following Java source code file:

```java
import java.util.List;
import java.util.ArrayList;

public class Example {
    public static void main(String[] args) {
        List<Integer> x = new ArrayList<Integer>();
        if (x.size() == 0) {}
    }
}
```

yields the following AST in Soufflé:

```prolog
$OrdinaryCompilationUnit(
  nil,
  [
    $SingleTypeImportDeclaration([List, [util, [java, nil]]]),
    [$SingleTypeImportDeclaration([ArrayList, [util, [java, nil]]]), nil]
  ],
  [
    $ClassDeclaration([$PublicModifier, nil], Example, nil, [
      $Method(
        [$PublicModifier, [$StaticModifier, nil]],
        $MethodHeader(
          $Void,
          $MethodDeclarator(
            main,
            nil,
            [
              $FormalParameter(
                nil,
                $ArrayType($ClassType(nil, String, nil), [
                  $Dimension(nil),
                  nil
                ]),
                $VariableDeclaratorId(args)
              ),
              nil
            ],
            nil
          ),
          nil
        ),
        $BlockStatement([
          $LocalVariableDeclarationStatement(
            $LocalVariableDeclaration(
              nil,
              $ClassType(nil, List, [$ClassType(nil, Integer, nil), nil]),
              [
                $VariableDeclarator($VariableDeclaratorId(x), [
                  $ClassInstanceCreationExpression(
                    nil,
                    $ClassInstanceCreationSubject(nil, ArrayList, [
                      $ClassType(nil, Integer, nil),
                      nil
                    ]),
                    nil,
                    nil
                  )
                ]),
                nil
              ]
            )
          ),
          [
            $IfThenStatement(
              $EqualsExpression(
                $MethodInvocationExpression(
                  $MethodInvocationIdentifiers([x, nil], nil, size),
                  nil
                ),
                $LiteralExpression($IntegerLiteral(0))
              ),
              $BlockStatement(nil)
            ),
            nil
          ]
        ])
      ),
      nil
    ]),
    nil
  ]
)
```

## License

MIT
