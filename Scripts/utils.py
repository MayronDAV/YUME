import sys
import os
import time
import urllib.request
from zipfile import ZipFile



def DownloadFile(url, filepath):
    path = filepath
    filepath = os.path.abspath(filepath)
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    
    if isinstance(url, list):
        for url_option in url:
            print("Downloading", url_option)
            try:
                DownloadFile(url_option, filepath)
                return
            except urllib.error.URLError as e:
                print(f"URL Error encountered: {e.reason}. Proceeding with backup...\n\n")
                os.remove(filepath)
                pass
            except urllib.error.HTTPError as e:
                print(f"HTTP Error  encountered: {e.code}. Proceeding with backup...\n\n")
                os.remove(filepath)
                pass
            except:
                print(f"Something went wrong. Proceeding with backup...\n\n")
                os.remove(filepath)
                pass
        raise ValueError(f"Failed to download {filepath}")
    if not isinstance(url, str):
        raise TypeError("Argument 'url' must be of type list or string")

    with open(filepath, 'wb') as f:
        headers = {'User-Agent': "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_4) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/83.0.4103.97 Safari/537.36"}
        req = urllib.request.Request(url, headers=headers)
        with urllib.request.urlopen(req) as response:
            total = response.length
            downloaded = 0
            startTime = time.time()
            while True:
                chunk = response.read(1024*1024)  # 1 MB chunks
                if not chunk:
                    break
                f.write(chunk)
                downloaded += len(chunk)
                
                try:
                    done = int(50 * downloaded / total) if downloaded < total else 50
                    percentage = (downloaded / total) * 100 if downloaded < total else 100
                except ZeroDivisionError:
                    done = 50
                    percentage = 100
                elapsedTime = time.time() - startTime
                try:
                    avgKBPerSecond = (downloaded / 1024) / elapsedTime
                except ZeroDivisionError:
                    avgKBPerSecond = 0.0

                avgSpeedString = '{:.2f} KB/s'.format(avgKBPerSecond)
                if avgKBPerSecond > 1024:
                    avgMBPerSecond = avgKBPerSecond / 1024
                    avgSpeedString = '{:.2f} MB/s'.format(avgMBPerSecond)
                sys.stdout.write('\r[{}{}] {:.2f}% ({})     '.format('█' * done, '.' * (50 - done), percentage, avgSpeedString))
                sys.stdout.flush()
    sys.stdout.write('\n')

def UnzipFile(filepath, deleteZipFile=True):
    zipFilePath = os.path.abspath(filepath) # get full path of files
    zipFileLocation = os.path.dirname(zipFilePath)

    zipFileContent = dict()
    zipFileContentSize = 0
    with ZipFile(zipFilePath, 'r') as zipFileFolder:
        for name in zipFileFolder.namelist():
            zipFileContent[name] = zipFileFolder.getinfo(name).file_size
        zipFileContentSize = sum(zipFileContent.values())
        extractedContentSize = 0
        startTime = time.time()
        for zippedFileName, zippedFileSize in zipFileContent.items():
            UnzippedFilePath = os.path.abspath(f"{zipFileLocation}/{zippedFileName}")
            os.makedirs(os.path.dirname(UnzippedFilePath), exist_ok=True)
            if os.path.isfile(UnzippedFilePath):
                zipFileContentSize -= zippedFileSize
            else:
                zipFileFolder.extract(zippedFileName, path=zipFileLocation, pwd=None)
                extractedContentSize += zippedFileSize
            try:
                done = int(50*extractedContentSize/zipFileContentSize)
                percentage = (extractedContentSize / zipFileContentSize) * 100
            except ZeroDivisionError:
                done = 50
                percentage = 100
            elapsedTime = time.time() - startTime
            try:
                avgKBPerSecond = (extractedContentSize / 1024) / elapsedTime
            except ZeroDivisionError:
                avgKBPerSecond = 0.0
            avgSpeedString = '{:.2f} KB/s'.format(avgKBPerSecond)
            if avgKBPerSecond > 1024:
                avgMBPerSecond = avgKBPerSecond / 1024
                avgSpeedString = '{:.2f} MB/s'.format(avgMBPerSecond)
            sys.stdout.write('\r[{}{}] {:.2f}% ({})     '.format('█' * done, '.' * (50-done), percentage, avgSpeedString))
            sys.stdout.flush()
    sys.stdout.write('\n')

    if deleteZipFile:
        os.remove(zipFilePath) # delete zip file
