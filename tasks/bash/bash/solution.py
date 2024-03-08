import argparse
import os

description = """Скрипт конкатенирует файлы наименьшего размера до тех пор, пока суммарный размер не превысит размеров блока файловой системы."""


def concatenate_files(output_path: str, input_path: str) -> int:
    with open(input_path, 'r') as input_file:
        with open(output_path, 'a') as output_file:
            output_file.write(input_file.read())

    return os.path.getsize(output_path)


def solve(output_path: str, source_dir: str, clear: bool) -> None:
    size_of_block = os.statvfs('/').f_bsize

    with open(output_path, 'w') as input_path:
        input_path.truncate()

    files = [os.path.join(source_dir, path) for path in os.listdir(source_dir)]
    files.sort(key=os.path.getsize)

    for input_path in files:
        if not os.path.isfile(input_path):
            continue

        input_size = os.path.getsize(input_path)
        output_size = os.path.getsize(output_path)
        new_output_path = concatenate_files(output_path, input_path)
        
        print(f"Added {input_path} {input_size}Б, ", end='')
        print(f"{output_path}: ", end='')
        print(f"{output_size} -> {new_output_path}")

        if os.path.getsize(output_path) > size_of_block:
            break

    if clear:
        print(f"Delete file {output_path} of size {os.path.getsize(output_path)}...")
        os.remove(output_path)


def main():
    parser = argparse.ArgumentParser(
        prog='Concatenator3000',
        description=description
    )

    parser.add_argument(
        '-p',
        '--path',
        default=None,
        help="Указывает, в какой файл будет производиться конкатенация"
    )
    parser.add_argument(
        '-c',
        '--clear',
        action='store_true',
        help="Удаляет файл-результат после работы скрипта"
    )
    parser.add_argument(
        '-s',
        '--source-path',
        default='.',
        help="Указывает, из какого каталога будут браться файлы"
    )

    args = parser.parse_args()

    if args.path is None:
        parser.print_help()
        return

    if not os.path.exists(args.source_path):
        print("Введена несуществующая директория")
        return

    solve(args.path, args.source_path, args.clear)


if __name__ == '__main__':
    main()
