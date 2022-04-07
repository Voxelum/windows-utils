export function getAppInstallerUri(): string
export function getWindowsVersion(): { major: number; minor: number; build: number } | undefined
export function setWindowBlur(hwnd: ArrayBuffer, effect: AccentState): boolean
export function setMica(hwnd: ArrayBuffer, enabled: boolean): boolean
export function createShortcut(exePath: string, destination: string, arguments: string, description: string, cwd: string, iconPath: string): boolean
export function getPackageFamilyName(): string
export function checkUpdateAvailabilityAsync(callback: (status: number) => void): boolean
export function initialize(): number

export interface InstallProgressHandler {
    (state: DeploymentProgressState, progress: number): void
}
export interface InstallCompleteHandler {
    (errorText: string): void
}
export function installUpdateByAppInstaller(appInstallerUri: string, progress: InstallProgressHandler, complete: InstallCompleteHandler): undefined | (() => void)

declare enum DeploymentProgressState {
    Queued = 0,
    Processing = 1,
}

declare enum AccentState {
    ACCENT_DISABLED = 0,
    ACCENT_ENABLE_GRADIENT = 1,
    ACCENT_ENABLE_TRANSPARENTGRADIENT = 2,
    ACCENT_ENABLE_BLURBEHIND = 3,
    ACCENT_ENABLE_ACRYLICBLURBEHIND = 4,
    ACCENT_ENABLE_HOSTBACKDROP = 5,
    ACCENT_INVALID_STATE = 6
}
