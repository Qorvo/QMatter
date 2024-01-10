import sys

import click
from intelhex import IntelHex


@click.command()
@click.option('--overlap', default='keep_last', help='Response to overlapping data in input',
              type=click.Choice(['error', 'keep_first', 'keep_last']))
@click.argument('output_hex', nargs=1)
@click.argument('input_hex', nargs=-1)
def main(overlap, output_hex, input_hex):
    if not input_hex:
        click.echo("Need at least one input hex file.", err=True)
        sys.exit(-1)

    if overlap == 'keep_first':
        overlap = 'ignore'
    elif overlap == 'keep_last':
        overlap = 'replace'

    output = IntelHex()
    output.start_addr = {'EIP': 0}
    for input_file in input_hex:
        input_data = IntelHex(input_file)
        output.merge(input_data, overlap=overlap)

    output.start_addr = {'EIP': 0}

    output.write_hex_file(output_hex)


if __name__ == '__main__':
    main()
