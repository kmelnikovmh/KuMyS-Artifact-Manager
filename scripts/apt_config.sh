#!/bin/bash

handle_error() {
    echo -e "\nError: $1" >&2
    exit 1
}

set -euo pipefail

if [ $# -ne 2 ]; then
    handle_error "Usage: $0 <nginx_ip> <nginx_port>"
fi

nginx_ip="$1"
nginx_port="$2"
base_dir="/etc/apt"

target_pattern="http://${nginx_ip}:${nginx_port}/"

found_files=$(find "$base_dir" -type f -name "ubuntu.sources" -path "*sources*" 2>/dev/null)

if [ -z "$found_files" ]; then
    handle_error "ubuntu.sources not found in $base_dir or subdirectories containing 'sources'" >&2
fi

for source_file in $found_files; do
    backup_file="${source_file}.bak-$(date +%s)"
    cp -f "$source_file" "$backup_file"
    
    awk -v ip="$nginx_ip" -v port="$nginx_port" -v pattern="$target_pattern" '
        /^[[:space:]]*#/ { print; next }
        
        $0 ~ /^[[:space:]]*URIs:[[:space:]]*http:\/\// {
            if ($0 ~ pattern) {
                print $0
                next
            }
            
            print "## " $0
            
            split($0, parts, /[[:space:]]*URIs:[[:space:]]*/)
            url = parts[2]
            
            sub(/^http:\/\//, "", url)
            
            slash_pos = index(url, "/")
            if (slash_pos == 0) {
                host = url
                path = ""
            } else {
                host = substr(url, 1, slash_pos - 1)
                path = substr(url, slash_pos)
            }
            
            split(host, domain_parts, /\./)
            seg_suffix = domain_parts[1]
            
            if (path == "" || path == "/") {
                new_path = "/" seg_suffix
            } else {
                if (path ~ /^\//) {
                    temp = substr(path, 2)  # Удалить ведущий /
                    slash_pos = index(temp, "/")
                    if (slash_pos == 0) {
                        first_segment = temp
                        rest = ""
                    } else {
                        first_segment = substr(temp, 1, slash_pos - 1)
                        rest = substr(temp, slash_pos)
                    }
                    new_path = "/" first_segment "_" seg_suffix rest
                } else {
                    new_path = "/" seg_suffix
                }
            }
            
            new_url = "http://" ip ":" port new_path
            
            print "URIs: " new_url
            next
        }
        { print }
    ' "$source_file" > "${source_file}.tmp" && mv "${source_file}.tmp" "$source_file"
    
    echo "Processed: $source_file (Backup: $backup_file)"
done

echo "All files updated successfully"