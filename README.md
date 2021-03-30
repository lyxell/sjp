# sjp

*sjp* is a Java parser for the Soufflé datalog engine. Its main
purpose is to enable program transformations and metaprogramming
of Java in Soufflé.

The parser is a work in progress but aims to be compliant with
the Java SE 15 grammar specification.

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
